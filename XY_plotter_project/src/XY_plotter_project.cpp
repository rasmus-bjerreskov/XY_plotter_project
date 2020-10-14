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
#include "DigitalIoPin.h"

#include "PenServoCtrl.h"

#include "Plotter.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
SemaphoreHandle_t uartMutex;
SemaphoreHandle_t sbRIT;
ParsedGdata_t data;

//limit switch data:
DigitalIoPin *LSWPin1;
DigitalIoPin *LSWPin2;
DigitalIoPin *LSWPin3;
DigitalIoPin *LSWPin4;
DigitalIoPin *Xstep;
DigitalIoPin *Ystep;
DigitalIoPin *Xdir;
DigitalIoPin *Ydir;


enum LSWLables {UP_LSW=0, RIGHT_LSW=1, DOWN_LSW=2, LEFT_LSW=3};
DigitalIoPin *limSws[4];

int x0, y0, y1, x1, dx, dy, D, i;
int prim1, prim2, prim3;
DigitalIoPin *primaryIo = NULL;
DigitalIoPin *secondaryIo = NULL;

Plotter *plotter = NULL;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void) {
	SystemCoreClockUpdate();
	Board_Init();
	heap_monitor_setup();

	ITM_init();



	// Set up Limit Switch Pins:
	LSWPin1 = new DigitalIoPin(1, 3, DigitalIoPin::pullup);
	LSWPin2 = new DigitalIoPin(0, 0, DigitalIoPin::pullup);
	LSWPin3 = new DigitalIoPin(0, 9, DigitalIoPin::pullup);
	LSWPin4 = new DigitalIoPin(0, 29, DigitalIoPin::pullup);
	Xstep = new DigitalIoPin(0,24,DigitalIoPin::output,true);
	Ystep = new DigitalIoPin(0,27,DigitalIoPin::output,true);
	Xdir = new DigitalIoPin(1,0,DigitalIoPin::output,true);
	Ydir = new DigitalIoPin(0,28,DigitalIoPin::output,true);
	plotter = new Plotter();

	Chip_RIT_Init(LPC_RITIMER);

	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


void RIT_IRQHandler(void)
{
 // This used to check if a context switch is required
 portBASE_TYPE xHigherPriorityWoken = pdFALSE;

 // Tell timer that we have processed the interrupt.
 // Timer then removes the IRQ until next match occurs
 Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag


 if(!plotter->getOffturn()) {

    if(i <= prim1) {
         plotter->switchOffturn();

               primaryIo->write(1);
                if (D > 0) {
                        secondaryIo->write(1);
                        D = D - 2* prim2;
                }
                D = D + 2* prim3;

        ++i;

    }
         else {
         Chip_RIT_Disable(LPC_RITIMER); // disable timer
         // Give semaphore and set context switch flag if a higher priority task was woken up
         i = 0;
         Xstep->write(0); // ?
         Ystep->write(0); // ?
         xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
         }
 }

 else {
	 plotter->switchOffturn();
     Xstep->write(0);
     Ystep->write(0);

 }

  // End the ISR and (possibly) do a context switch
         portEND_SWITCHING_ISR(xHigherPriorityWoken);

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
	sbRIT = xSemaphoreCreateBinary();

	xTaskCreate(vParserTask, "vParserTask",
	configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t*) NULL);

	vTaskStartScheduler();

	return 1;
}

