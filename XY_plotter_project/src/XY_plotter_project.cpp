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
#include "PenServoCtrl.h"

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

ParsedGdata_t *parsedData;
PenServoController *penServo;
Plotter *plotter;
Parser *parser;

enum class RelModes {
	REL, ABS
};

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

	plotter = new Plotter();

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
	parsedData = new ParsedGdata_t;

// mock values for testing the mdraw communication:
	parsedData->limitSw[0] = 1;
	parsedData->limitSw[1] = 1;
	parsedData->limitSw[2] = 1;
	parsedData->limitSw[3] = 1;
	parsedData->Adir = 0;
	parsedData->Bdir = 0;
	parsedData->penUp = 160;
	parsedData->penDown = 90;
	parsedData->penCur = parsedData->penUp;
	parsedData->speed = 80;
	parsedData->canvasLimits.Xmm = 150;
	parsedData->canvasLimits.Ymm = 100; //TODO sync these with plotter values

	binPen = xSemaphoreCreateBinary();
	qCmd = xQueueCreate(1, sizeof(PlotInstruct_t));
	penServo = new PenServoController(parsedData);
	parser = new Parser();

	char str[MAX_STR_LEN + 1];
	char *lf = 0;

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
		ITM_write("Received: ");
		ITM_write(buf);
		//if command was recognised and parsed, put it on queue
		if (parser->parse(parsedData, buf)) {
			PlotInstruct_t instruct {

					{ parsedData->PenXY.Xmm, parsedData->PenXY.Ymm, 0, 0 }, parsedData->codeType,
					parsedData->penCur };
			xQueueSend(qCmd, &instruct, portMAX_DELAY);
		}
	}
}

/*Execute hardware instructions*/
void plotter_task(void *pvParameters) {
	PlotInstruct_t instrBuf;

	xEventGroupSync(eGrp, PLOT_b, TASK_BITS, portMAX_DELAY);
	while (1) {
		//TODO a flag should be set to avoid the same command being command being read more than once
		xQueuePeek(qCmd, &instrBuf, portMAX_DELAY); //get data and send it on to tx task
		/*set flag here so that sending
		 reply can happen concurrently with executing instructions*/
		xEventGroupSetBits(eGrp, PLOT_b);

		switch (instrBuf.code) {
		case (GcodeType::M10):
			penServo->updatePos(parsedData->penUp);
			break;

		case (GcodeType::M1):
			penServo->updatePos(instrBuf.penPos);
			break;

		case (GcodeType::M2):
			//TODO no action needed here, possibly...
			break;

		case (GcodeType::M5):
			plotter->setCanvasSize(parsedData->canvasLimits.Xmm, parsedData->canvasLimits.Ymm);
			break;

		case (GcodeType::G28): {

			int penCur = instrBuf.penPos;
			parsedData->penCur = parsedData->penUp;

			xSemaphoreGive(binPen);
			plotter->plotLine(0, 0);
			parsedData->penCur = penCur;
			xSemaphoreGive(binPen);
		}
			break;

		case (GcodeType::G1):
			umsToSteps(&(parsedData->PenXY),
					parsedData->relativityMode ? RelModes::REL : RelModes::ABS);
			plotter->plotLine(parsedData->PenXY.Xsteps, parsedData->PenXY.Ysteps);
			break;

		default:
			// No laser (M4) implemented...
			break;
		}
	}
}

void pen_task(void *param) {
	xEventGroupSync(eGrp, PEN_b, TASK_BITS, portMAX_DELAY);

	while (1) {
		xSemaphoreTake(binPen, portMAX_DELAY);
		penServo->updatePos(parsedData->penCur); //TODO change semaphore to queue holding pen value
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
					parsedData->canvasLimits.Xmm, parsedData->canvasLimits.Ymm, parsedData->Adir,
					parsedData->Bdir, parsedData->speed, parsedData->penUp, parsedData->penDown); // these need to be fixed too, like with M11 CDM,
																		  // to use plotter instead of data
																		  // M2 and M5, which are currently broken,
																		  // should affect which info is sent out
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

	xTaskCreate(pen_task, "pen", configMINIMAL_STACK_SIZE, NULL,
	tskIDLE_PRIORITY + 2, NULL);

	xTaskCreate(send_task, "tx", configMINIMAL_STACK_SIZE, NULL,
	tskIDLE_PRIORITY + 1UL, (TaskHandle_t*) NULL);

	xTaskCreate(cdc_task, "CDC",
	configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	eGrp = xEventGroupCreate();

	vTaskStartScheduler();

	return 1;
}

