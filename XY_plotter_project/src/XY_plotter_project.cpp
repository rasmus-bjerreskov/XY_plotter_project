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
#include "heap_lock_monitor.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

volatile SemaphoreHandle_t receiverNotifier;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	heap_monitor_setup();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

/* LED1 toggle thread */
static void vParserTask(void *pvParameters) {
	std::regex g1("G1 X[-+]?[0-9]*[.][0-9]+ Y[-+]?[0-9]*[.][0-9]+ A[0-1]");
	std::regex g28("G28");
	std::regex m1("M1 ([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
	std::regex m4("M4 ([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
	std::regex m5("M5 A0 B0 H310 W380 S80");
	std::regex m10("M10");

	std::string code;

	while (1) {

		xSemaphoreTake(receiverNotifier), portMAX_DELAY);

		code = receiver.getCodeLine();

		std::printf("%s\r\n",code.c_str());

		if (code[0] == 'G') {
			if (code[1] == '1') {
				if (std::regex_match(code, g1)) {
					receiver.sendACK();
				//	std::printf("coords to go to = X:%s Y:%s\r\n",
				//				str.substr(str.find("X") + 1 , (str.substr(str.find("X") + 1)).find(" ")).c_str(),
				//				str.substr(str.find("Y") + 1, (str.substr(str.find("Y") + 1)).find(" ")).c_str());
				}
				else {
					receiver.sendError();
				}
			}
			else if (code[1] == '2' && code[2] == '8') {
				if (std::regex_match(code, g28)) {
					receiver.sendACK();
					//std::printf("going to origin\r\n");
				}
				else {
					receiver.sendError();
				}
			}
		}
		else if (code[0] == 'M') {
			if (code[1] == '1' && code[2] == '0') {
				if (std::regex_match(code, m10)) {
					receiver.sendACK();
					// ("COM-port opened\r\n");
				}
				else {
					receiver.sendError();
				}
			}
			else if (code[1] == '1') {
				if (std::regex_match(code, m1)) {
					receiver.sendACK();
					// std::printf("penposition = %s\r\n", str.substr(str.find(" ") + 1).c_str());
				}
				else {
					receiver.sendError();
				}
			}
			else if (code[1] == '4') {
				if (std::regex_match(code, m4)) {
					receiver.sendACK();
					// std::printf("penpower = %s\r\n", str.substr(str.find(" ") + 1).c_str());
				}
				else {
					receiver.sendError();;
				}
			}
			else if (code[1] == '5') {
				if (std::regex_match(str, m5)) {
					receiver.sendACK();
					// std::printf("plotter setting setted");
				}
				else {
					receiver.sendError();
				}
			}

		}
		else {
			receiver.sendError();
		}
	}
}

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
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
int main(void)
{
	prvSetupHardware();

	receiverNotifier = xCreateCountingSemaphore();

	/* LED1 toggle thread */
	xTaskCreate(vParserTask, "vParserTask",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

