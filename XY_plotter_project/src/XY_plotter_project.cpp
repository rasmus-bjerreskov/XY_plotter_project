/*
 ===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

#include <cstring>

#include "ITM_write.h"
#include "DigitalIoPin.h"
#include "heap_lock_monitor.h"
#include "user_vcom.h"

#include "ParsedGdata.h"
#include <global_semphrs.h>
#include "GcodePipe.h"
#include "SimpleUARTWrapper.h"
#include "Parser.h"
#include "MockPipe.h"

#include "Plotter.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
QueueHandle_t qCmd; //Holds commands to hardware functions
EventGroupHandle_t eGrp;
SemaphoreHandle_t binPen;

ParsedGdata_t *systemData;  // this should NOT be used anymore...
// TODO move all non volatile data of the system inside Plotter

Plotter *plotter;
Parser *parser;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void laser_disable() {
	Chip_SCT_Init(LPC_SCT1);
	Chip_SCTPWM_Stop(LPC_SCT1);

	LPC_SCT1->OUT[1].SET = 0; // event 2 has no effect on  SCTx_OUT1 --> laser is always off
	LPC_SCT1->MATCHREL[1].H = 1;
	Chip_SWM_MovablePinAssign(SWM_SCT1_OUT0_O, 12);
	Chip_SCTPWM_Start(LPC_SCT1);
}

/* Sets up system hardware */
static void prvSetupHardware(void) {
	SystemCoreClockUpdate();
	Board_Init();
	heap_monitor_setup();

	ITM_init();

	Chip_RIT_Init(LPC_RITIMER);

	NVIC_SetPriority(RITIMER_IRQn,
	configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void umsToSteps(CanvasCoordinates_t *coords, RelModes mode) {
	coords->Xsteps = coords->Xum / UMS_PER_STEP;
	coords->Ysteps = coords->Yum / UMS_PER_STEP;

	switch (mode) {
	case RelModes::REL:
		coords->Xsteps += plotter->penXYPos.Xsteps;
		coords->Ysteps += plotter->penXYPos.Ysteps;
		break;

	case RelModes::ABS:
		break;

	}
}

/*Receive G-code lines from mDraw, validate and parse code into hardware instructions*/
static void parse_task(void *pvParameters) {
	ParsedGdata_t parsedData;

	systemData = new ParsedGdata_t;
	systemData->limitSw[0] = 1;
	systemData->limitSw[1] = 1;
	systemData->limitSw[2] = 1;
	systemData->limitSw[3] = 1;
	systemData->Adir = 0;
	systemData->Bdir = 0;
	systemData->penUp = 160;
	systemData->penDown = 90;
	systemData->penCur = systemData->penUp;
	systemData->speed = 80;
	systemData->canvasLimits.Xmm = 150;
	systemData->canvasLimits.Ymm = 100;

	plotter = new Plotter(90, 160, 160);

	binPen = xSemaphoreCreateBinary();
	qCmd = xQueueCreate(1, sizeof(PlotInstruct_t));
	parser = new Parser();

	char str[MAX_STR_LEN + 1];
	char *lf = 0;
	uint32_t instructCnt = 0;
	xEventGroupSync(eGrp, RX_b, TASK_BITS, portMAX_DELAY);

	while (1) {
		int len = 0;
		char buf[MAX_STR_LEN + 1];
		char *curBufPos = buf;
		//receive-append strings
		do {
			len = USB_receive((uint8_t*) str, MAX_STR_LEN);
			str[len] = 0;
			lf = strchr(str, '\n');

			memcpy(buf, str, len);
			curBufPos += len;

		} while (lf == NULL);

		*curBufPos = 0;
		ITM_write("\nReceived: ");
		ITM_write(buf);
		//if command was recognised and parsed, put it on queue
		if (parser->parse(&parsedData, buf)) {
			instructCnt++;

			switch (parsedData.codeType) {
			case (GcodeType::M2):
				systemData->penUp = parsedData.penUp;
				systemData->penDown = parsedData.penDown;
				break;

			case (GcodeType::M5):
				systemData->Adir = parsedData.Adir;
				systemData->Bdir = parsedData.Bdir;
				systemData->canvasLimits.Xmm = parsedData.canvasLimits.Xmm;
				systemData->canvasLimits.Ymm = parsedData.canvasLimits.Ymm;
				systemData->speed = parsedData.speed;
				break;

			default:
				break;
			}

			PlotInstruct_t instruct { { parsedData.PenXY.Xum,
					parsedData.PenXY.Yum, 0, 0 }, parsedData.codeType,
					parsedData.penCur, parsedData.relativityMode, instructCnt };
			xQueueSend(qCmd, &instruct, portMAX_DELAY);
			xEventGroupSetBits(eGrp, TX_b);
		}
	}
}

/*Execute hardware instructions*/
void plotter_task(void *pvParameters) {
	PlotInstruct_t instrBuf;
	char str[40];

	xEventGroupSync(eGrp, PLOT_b, TASK_BITS, portMAX_DELAY);
	while (1) {
		//Wait for message that new item is on queue. Without this, same instruction may be read multiple times
		xEventGroupWaitBits(eGrp, TX_b, pdTRUE, pdTRUE, portMAX_DELAY);
		xQueuePeek(qCmd, &instrBuf, portMAX_DELAY); //get data and send it on to tx task

		sprintf(str, "Instr #%d\n", instrBuf.cnt);
		ITM_write(str);

		/*set flag here so that sending
		 reply can happen concurrently with executing instructions*/
		xEventGroupSetBits(eGrp, PLOT_b);

		switch (instrBuf.code) {
		case (GcodeType::M10):
			penServo->updatePos(systemData->penUp); // TODO replace with plotter->initPenServo() or something similar;
			systemData->penCur = penServo->getCurVal();  // TODO get rid of this:   all non volatile data go into Plotter
																			     // from now on
			break;

		case (GcodeType::M1):
			penServo->updatePos(instrBuf.penPos);  // TODO replace this with  with plotter->updatePos() or something similar
			systemData->penCur = penServo->getCurVal(); // TODO get rid of this:       all non volatile data go into Plotter
																					// from now on
			break;

		case (GcodeType::M5):
			plotter->setCanvasSize(systemData->canvasLimits.Xmm,	// TODO all non volatile data are in Plotter,
					systemData->canvasLimits.Ymm);					// get canvas size from Plotter instead...
			break;

		case (GcodeType::G28): {
			// TODO fix this: (see above)
			int penCur = penServo->getCurVal();
			systemData->penCur = systemData->penUp;

			xSemaphoreGive(binPen);
			plotter->plotLine(0, 0);
			systemData->penCur = penCur;
			xSemaphoreGive(binPen);
		}
			break;

		case (GcodeType::G1):
			umsToSteps(&(instrBuf.newPos), instrBuf.relMode);
			plotter->plotLine(instrBuf.newPos.Xsteps, instrBuf.newPos.Ysteps);
			break;

		default:
			// No laser (M4) implemented...
			// M2 is handled in parser_task
			break;
		}
	}
}

/*Send OK and other required data to mDraw*/
void send_task(void *pvParameters) {
	char str[80];
	PlotInstruct_t instrBuf;

	xEventGroupSync(eGrp, TX_b, TASK_BITS, portMAX_DELAY);
	while (1) {
		xEventGroupWaitBits(eGrp, PLOT_b, pdTRUE, pdTRUE, portMAX_DELAY);
		xQueueReceive(qCmd, &instrBuf, portMAX_DELAY);

		switch (instrBuf.code) {

		case (GcodeType::M10):
			sprintf(str,
					"M10 XY %d %d 0.00 0.00 A%d B%d H0 S%d U%d D%d\r\nOK\r\n",
					systemData->canvasLimits.Xmm, systemData->canvasLimits.Ymm,
					systemData->Adir, systemData->Bdir, systemData->speed,
					systemData->penUp, systemData->penDown);
			break;

		case (GcodeType::M11):
			sprintf(str, "M11 %d %d %d %d\r\nOK\r\n", plotter->LSWPin1->read(),
					plotter->LSWPin2->read(), plotter->LSWPin3->read(),
					plotter->LSWPin4->read());
			break;

		default:
			strcpy(str, "OK\r\n");
			break;
		}
		ITM_write("Reply: ");
		ITM_write(str);
		USB_send((uint8_t*) str, strlen(str));

	}
}

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats(void) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void) {
	prvSetupHardware();

//laser_disable(); //firmware does not support laser operation
	DigitalIoPin laser(0, 12, DigitalIoPin::output, true);
	laser.write(false);

	xTaskCreate(parse_task, "rx",
	configMINIMAL_STACK_SIZE * 2, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	xTaskCreate(plotter_task, "plot", configMINIMAL_STACK_SIZE * 3, NULL,
	tskIDLE_PRIORITY + 1UL, (TaskHandle_t*) NULL);

	xTaskCreate(send_task, "tx", configMINIMAL_STACK_SIZE, NULL,
	tskIDLE_PRIORITY + 1UL, (TaskHandle_t*) NULL);

	xTaskCreate(cdc_task, "CDC",
	configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	eGrp = xEventGroupCreate();

	vTaskStartScheduler();

	return 1;
}

