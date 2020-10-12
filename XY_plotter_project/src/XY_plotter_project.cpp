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
#include "heap_lock_monitor.h"

#include "ParsedGdata.h"
#include "GcodePipe.h"
#include "SimpleUARTWrapper.h"
#include "Parser.h"
#include "MockPipe.h"
#include "ITM_write.h"

#include "PenServoCtrl.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
SemaphoreHandle_t uartMutex;
ParsedGdata_t data;

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
static void vParserTask(void *pvParameters) {
	data.canvasLimits.Y = 380;
	data.canvasLimits.X = 310;

	// mock values for testing the mdraw communication:
	data.limitSw[0] = 1;
	data.limitSw[1] = 1;
	data.limitSw[2] = 1;
	data.limitSw[3] = 1;
	data.Adir = 1;
	data.Bdir = 1;
	data.penUp = 150;
	data.penDown = 90;
	data.penCur = 150;

	PenServoController *penServo = new PenServoController(&data);
	data.speed = 80;
	SimpleUART_Wrapper pipe(uartMutex);
	//MockPipe pipe;
	//Parser parser(&pipe);
	char str[50];
	int c;
	//servo test code
	while (1) {
		Board_LED_Set(1, true);
		Board_LED_Set(0, false);
		data.penCur = 150;
		penServo->updatePos();
		vTaskDelay(3000);
		data.penCur = 90;
		penServo->updatePos();
		vTaskDelay(3000);
		Board_LED_Set(1, false);
		Board_LED_Set(0, true);
		data.penCur = 140; //illegal value
		penServo->updatePos();
		vTaskDelay(3000);
	}
	/*SimpleUART_Wrapper pipe(uartMutex);
	 //MockPipe pipe;
	 Parser parser(&pipe);
	 char str[50];
	 int c;

	 int i = 5;

	 while (1) {
	 if (parser.parse(&data)){
	 switch(data.codeType) {
	 case GcodeType::G1:
	 case GcodeType::G28:
	 case GcodeType::M1:
	 case GcodeType::M2:
	 case GcodeType::M4:
	 case GcodeType::M5:
	 pipe.sendAck();
	 break;
	 case GcodeType::M10:
	 snprintf(str, 50, "M10 XY %d %d 0.00 0.00 A%d B%d H0 S%d U%d D%d\r\n",
	 data.canvasLimits.X,
	 data.canvasLimits.Y,
	 data.Adir,
	 data.Bdir,
	 data.speed,
	 data.penServo->up,
	 data.penServo->down);
	 pipe.sendAck();
	 break;
	 case GcodeType::M11:
	 snprintf(str, 50, "M11 %d %d %d %d\r\n",
	 data.limitSw[0],
	 data.limitSw[1],
	 data.limitSw[2],
	 data.limitSw[4]);
	 pipe.sendLine(str);
	 pipe.sendAck();
	 break;
	 }
	 pipe.sendAck();
	 }
	 else {
	 ITM_write("Problem occured\r\n");
	 //vTaskDelay(portMAX_DELAY);
	 }
	 }*/

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

	uartMutex = xSemaphoreCreateMutex();

	xTaskCreate(vParserTask, "vParserTask",
	configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	vTaskStartScheduler();

	return 1;
}

