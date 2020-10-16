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

#include <cstring>

#include "ITM_write.h"
#include "heap_lock_monitor.h"
#include "user_vcom.h"

#include "ParsedGdata.h"
#include "GcodePipe.h"
#include "SimpleUARTWrapper.h"
#include "Parser.h"
#include "MockPipe.h"
#include "PenServoCtrl.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
QueueHandle_t qCmd;
SemaphoreHandle_t binAck;

ParsedGdata_t *data;
PenServoController *penServo;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void) {
	SystemCoreClockUpdate();
	Board_Init();
	heap_monitor_setup();

	ITM_init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

/* The parser task */
static void parse_task(void *pvParameters) {
	data = new ParsedGdata_t;
	data->canvasLimits.Y = 380;
	data->canvasLimits.X = 310;

	// mock values for testing the mdraw communication:
	data->limitSw[0] = 1;
	data->limitSw[1] = 1;
	data->limitSw[2] = 1;
	data->limitSw[3] = 1;
	data->Adir = 1;
	data->Bdir = 1;
	data->penUp = 150;
	data->penDown = 90;
	data->penCur = 150;
	data->speed = 80;

	qCmd = xQueueCreate(20, sizeof(PlotInstruct_t));
	binAck = xSemaphoreCreateBinary();

	penServo = new PenServoController(data);

	const int MAX_LEN = 50;
	char str[MAX_LEN + 1];
	char *lf = 0;

	while (1) {
		int len = 0;
		char buf[MAX_LEN + 1];
		char *curBufPos = buf;
		//receive-append strings
		do {
			len = USB_receive((uint8_t*) str, MAX_LEN);
			str[len] = 0;
			lf = strchr(str, '\n');

			memcpy(buf, str, len);
			curBufPos += len;

		} while (lf == NULL);

		*curBufPos = 0;
		ITM_write("Received: ");
		ITM_write(buf);

		PlotInstruct_t instruct { { data->PenXY.X, data->PenXY.Y },
				data->codeType, data->penCur };
		xQueueSend(qCmd, &instruct, portMAX_DELAY);

		//USB_send((uint8_t*)"OK\r\n", 4);

	}
}

void plotter_task(void *pvParameters) {
	PlotInstruct_t instrBuf;
	while (1) {
		xQueuePeek(qCmd, &instrBuf, portMAX_DELAY); //get data and send it on to tx task
		/*give semaphore here so that sending
		 reply can happen concurrently with plotting*/
		xSemaphoreGive(binAck);

		switch (instrBuf.code) {
		case GcodeType::M10:
			//do init
			break;

		case GcodeType::M1:
			penServo->updatePos();
			break;

		case GcodeType::G28:
			//go to origin
			break;

		case GcodeType::G1:
			//go to coords
			break;

		default:
			break;
		}
	}
}

void send_task(void *pvParameters) {
	char str[80];
	PlotInstruct_t instrBuf;

	while (1) {
		xSemaphoreTake(binAck, portMAX_DELAY);
		xQueueReceive(qCmd, &instrBuf, portMAX_DELAY);

		switch (instrBuf.code) {

		case GcodeType::M10:
			sprintf(str,
					"M10 XY %d %d 0.00 0.00 A%d B%d H0 S%d U%d D%d\r\nOK\r\n",
					data->canvasLimits.X, data->canvasLimits.Y, data->Adir,
					data->Bdir, data->speed, data->penUp, data->penDown);
			break;

		case GcodeType::M11:
			sprintf(str, "M11 %d %d %d %d\r\nOK\r\n", data->limitSw[0],
					data->limitSw[1], data->limitSw[2], data->limitSw[3]);
			break;

		default:
			strcpy(str, "OK\r\n");

			USB_send((uint8_t*)str, strlen(str));

		}

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

	xTaskCreate(parse_task, "rx",
	configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	xTaskCreate(plotter_task, "plot", configMINIMAL_STACK_SIZE, NULL,
	tskIDLE_PRIORITY + 1UL, (TaskHandle_t*) NULL);

	xTaskCreate(send_task, "tx", configMINIMAL_STACK_SIZE, NULL,
	tskIDLE_PRIORITY + 1UL, (TaskHandle_t*) NULL);

	xTaskCreate(cdc_task, "CDC",
	configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	vTaskStartScheduler();

	return 1;
}

