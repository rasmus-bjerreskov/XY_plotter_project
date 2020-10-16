/*
 * Plotter.cpp
 *
 *  Created on: 14.10.2020
 *      Author: Santeri Rauhala
 */

#include "Plotter.h"
#include <stdlib.h>

static Plotter *plotter; // used to hold the only plotter

// in constructor the IO-pins and their settings are set, also some starting values
Plotter::Plotter() {

	plotter = this;

	offturn = false;

	LSWPin1 = new DigitalIoPin(1, 3, DigitalIoPin::pullup);
	LSWPin2 = new DigitalIoPin(0, 0, DigitalIoPin::pullup);
	LSWPin3 = new DigitalIoPin(0, 9, DigitalIoPin::pullup);
	LSWPin4 = new DigitalIoPin(0, 29, DigitalIoPin::pullup);
	Xstep = new DigitalIoPin(0,24,DigitalIoPin::output,true);
	Ystep = new DigitalIoPin(0,27,DigitalIoPin::output,true);
	Xdir = new DigitalIoPin(1,0,DigitalIoPin::output,true);
	Ydir = new DigitalIoPin(0,28,DigitalIoPin::output,true);

	sbRIT = xSemaphoreCreateBinary();

}

Plotter::~Plotter() {
	// TODO Auto-generated destructor stub
}

// used to determine when to write 0 and when to write 1 to stepper motors
void Plotter::switchOffturn() {
	offturn = !offturn;
}

// used to determine which turn it is, to write 0 or write 1 to stepper motors
bool Plotter::getOffturn() {
	return offturn;
}

//used to plot the line with RIT_IRQHandlerer
void Plotter::plotLine(int x1_l, int y1_l, int us)
{
 // used to determine the frequency of the steps
 uint64_t cmp_value;
 // Determine approximate compare value based on clock rate and passed interval
 cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
 // disable timer during configuration
 Chip_RIT_Disable(LPC_RITIMER);

 x1 = x1_l;
 y1 = y1_l;
 i = 0;
 dx = abs(x1 - penXYPos.Xsteps);
 dy = abs(y1 - penXYPos.Ysteps);

     if (dx > dy) { // when x is dominant axis
         prim1 = x1;
         prim2 = dx;
         prim3 = dy;
         primaryIo = Xstep;
         secondaryIo = Ystep;
         D = 2*dy - dx;
     }
     else {	// when y is dominant axis
         prim1 = y1;
         prim2 = dy;
         prim3 = dx;
         primaryIo = Ystep;
         secondaryIo = Xstep;
         D = 2*dx - dy;
     }

 // enable automatic clear on when compare value==timer value
 // this makes interrupts trigger periodically
 Chip_RIT_EnableCompClear(LPC_RITIMER);
 // reset the counter
 Chip_RIT_SetCounter(LPC_RITIMER, 0);
 Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
 // start counting
 Chip_RIT_Enable(LPC_RITIMER);
 // Enable the interrupt signal in NVIC (the interrupt controller)
 NVIC_EnableIRQ(RITIMER_IRQn);
 // wait for ISR to tell that we're done
 if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
 // Disable the interrupt signal in NVIC (the interrupt controller)
 NVIC_DisableIRQ(RITIMER_IRQn);
 }
 else {
 // unexpected error
 }
}


//used to plot the line with RIT_IRQHandlerer
void Plotter::plotLine(int x0, int y0, int x1, int y1, int us)
{
 // used to determine the frequency of the steps
 uint64_t cmp_value;
 // Determine approximate compare value based on clock rate and passed interval
 cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
 // disable timer during configuration
 Chip_RIT_Disable(LPC_RITIMER);
 i = 0;
 dx = abs(x1 - x0);
 dy = abs(y1 - y0);

     if (dx > dy) { // when x is dominant axis
         prim1 = x1;
         prim2 = dx;
         prim3 = dy;
         primaryIo = Xstep;
         secondaryIo = Ystep;
         D = 2*dy - dx;
     }
     else {	// when y is dominant axis
         prim1 = y1;
         prim2 = dy;
         prim3 = dx;
         primaryIo = Ystep;
         secondaryIo = Xstep;
         D = 2*dx - dy;
     }

 // enable automatic clear on when compare value==timer value
 // this makes interrupts trigger periodically
 Chip_RIT_EnableCompClear(LPC_RITIMER);
 // reset the counter
 Chip_RIT_SetCounter(LPC_RITIMER, 0);
 Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
 // start counting
 Chip_RIT_Enable(LPC_RITIMER);
 // Enable the interrupt signal in NVIC (the interrupt controller)
 NVIC_EnableIRQ(RITIMER_IRQn);
 // wait for ISR to tell that we're done
 if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
 // Disable the interrupt signal in NVIC (the interrupt controller)
 NVIC_DisableIRQ(RITIMER_IRQn);
 }
 else {
 // unexpected error
 }
}

// used to determen which limit switch is which and also the size of the canvas and length of step and go to starting position (0,0)
void Plotter::calibrateCanvas() {
	int stepCount = 0;

	// calibrate XMotor:
	// Drive X-motor to left until a limit switch is hit:
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read() && LSWPin4->read()) {
		Plotter::plotLine(1, 0, 0, 0, 2);
	}

	// Record which limit switch was hit:
	limSws[LEFT_LSW] = (!LSWPin1->read())?
			 	 	 	 LSWPin1 :
						 ((!LSWPin2->read())?
						    LSWPin2 :
						    ((!LSWPin3->read())?
							   LSWPin3 :
							   LSWPin4));

	// Drive to right one step:
	Plotter::plotLine(0, 0, 1, 0, 2);
	// Drive to right until the left limit switch opens, count the steps:
	while (!(limSws[LEFT_LSW]->read())) {
		Plotter::plotLine(0, 0, 1, 0, 2);
		stepCount++;
	}

	// Drive the X-motor to the right until another limit Switch is hit:
	// count all the steps while driving
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read() && LSWPin4->read()) {
		Plotter::plotLine(0, 0, 1, 0, 2);
		stepCount++;
	}

	// decrease step count by 1:
	stepCount--;
	canvasSize.Xsteps = stepCount;	// should this be stepCount+1 ?!
	// set the current XPos:
	penXYPos.Xsteps = 0; // The left side is the zero coordinate

	// Record which limit switch it was:
	limSws[RIGHT_LSW] = (!LSWPin1->read())?
						  LSWPin1 :
						  ((!LSWPin2->read())?
						     LSWPin2 :
							 ((!LSWPin3->read())?
							    LSWPin3 :
							    LSWPin4));

	// Drive to left until the right limit switch opens:
	// Increase XPos accordingly:
	while (!(limSws[RIGHT_LSW]->read())) {
		Plotter::plotLine(0, 0, 1, 0, 2);
		penXYPos.Xsteps++;
	}

	// Reset the step count:
	stepCount = 0;

	// calibrate YMotor:
	// Drive Y-motor to down until a limit switch is hit:
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read() && LSWPin4->read()) {
		Plotter::plotLine(0, 0, 0, 1, 2);
	}

	// Record which limit switch was hit:
	limSws[DOWN_LSW] = (!LSWPin1->read())?
						 LSWPin1 :
						 ((!LSWPin2->read())?
						    LSWPin2 :
						    ((!LSWPin3->read())?
						       LSWPin3 :
							   LSWPin4));

	// Drive one step up:
	Plotter::plotLine(0, 1, 0, 0, 2);

	// Drive up until the down limit switch opens:
	// count the steps:
	while (!(limSws[LSWLables::DOWN_LSW]->read())) {
		Plotter::plotLine(0, 1, 0, 0, 2);
		stepCount++;
	}

	// Drive the Y-motor upwards until another limit Switch is hit:
	// count all the steps while driving
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read() && LSWPin4->read()) {
		Plotter::plotLine(0, 1, 0, 0, 2);
		stepCount++;
	}

	// Record which limit switch it was:
	limSws[UP_LSW] = (!LSWPin1->read())?
					   LSWPin1 :
					   ((!LSWPin2->read())?
						  LSWPin2 :
						  ((!LSWPin3->read())?
						     LSWPin3 :
						     LSWPin4));

	// decrease step count by 1:
	stepCount--;
	canvasSize.Ysteps = stepCount;	// should this be stepCount+1 ?!
	// set the current YPos:
	penXYPos.Ysteps = 0;


	// Drive downwards until the upper limit switch opens:
	// increase YPos accordingly:
	while (!(limSws[UP_LSW]->read())) {
		Plotter::plotLine(0, 0, 0, 1, 2);
		penXYPos.Ysteps++;
	}

	// drive to the center of the canvas:
	Plotter::plotLine(canvasSize.Xsteps/2, canvasSize.Ysteps/2, 2);
	penXYPos.Xsteps = canvasSize.Xsteps/2;
	penXYPos.Ysteps = canvasSize.Ysteps/2;
}

// using Bresenham's line algorithm to determine when to step with dominant axis only and when with both
void Plotter::isr(portBASE_TYPE xHigherPriorityWoken){
    if (!offturn) { // when to write 1 on steppers

                if (i <= prim1) { // run as long as i = x0/y0 is smaller or equal to x1/y1
                    offturn = !offturn;

                    primaryIo->write(1);
                    if (D > 0) {
                        secondaryIo->write(1);
                        D = D - 2 * prim2;
                    }
                    D = D + 2 * prim3;

                    ++i;

                } else {	// when i reaches x1/y1 disable the timer and reset i

                    Chip_RIT_Disable(LPC_RITIMER); // disable timer
                    i = 0;
                    Xstep->write(0);
                    Ystep->write(0);
                    // Give semaphore and set context switch flag if a higher priority task was woken up
                    xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
                }
            }

    else { // when to write 0 on the steppers
    	switchOffturn();
        Xstep->write(0);
        Ystep->write(0);

    }
}

extern "C" {
void RIT_IRQHandler(void) {
    // This used to check if a context switch is required
    portBASE_TYPE xHigherPriorityWoken = pdFALSE;

    // Tell timer that we have processed the interrupt.
    // Timer then removes the IRQ until next match occurs
    Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
    if (plotter != nullptr) {

        plotter->isr(xHigherPriorityWoken); // plot with isr-function

        // End the ISR and (possibly) do a context switch
        portEND_SWITCHING_ISR(xHigherPriorityWoken);
    }

}
}
