/*
 * Plotter.cpp
 *
 *  Created on: 14.10.2020
 *      Author: Santeri Rauhala
 */

#include "Plotter.h"
#include <stdlib.h>


Plotter::Plotter() {
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

void Plotter::switchOffturn() {
	offturn = !offturn;
}

bool Plotter::getOffturn() {
	return offturn;
}

void Plotter::plotLine(int x0_l, int y0_l, int x1_l, int y1_l, int us)
{
 uint64_t cmp_value;
 // Determine approximate compare value based on clock rate and passed interval
 cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
 // disable timer during configuration
 Chip_RIT_Disable(LPC_RITIMER);

 x0 = x0_l;
 x1 = x1_l;
 y0 = y0_l;
 y1 = y1_l;
 i = 0;
 dx = abs(x1 - x0);
 dy = abs(y1 - y0);

     if (dx > dy) {
         prim1 = x1;
         prim2 = dx;
         prim3 = dy;
         primaryIo = Xstep;
         secondaryIo = Ystep;
         D = 2*dy - dx;
     }
     else {
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

void Plotter::calibrateCanvas() {
	// for remembering where we are at the canvas:
	int xPos = 0;
	int yPos = 0;
	int stepCount = 0;
	int xSteps = 0;
	int ySteps = 0;

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
	xSteps = stepCount;	// should this be stepCount+1 ?!
	// set the current XPos:
	xPos = stepCount;

	// Record which limit switch it was:
	limSws[RIGHT_LSW] = (!LSWPin1->read())?
						  LSWPin1 :
						  ((!LSWPin2->read())?
						     LSWPin2 :
							 ((!LSWPin3->read())?
							    LSWPin3 :
							    LSWPin4));

	// Drive to left until the right limit switch opens:
	// Decrease XPos accordingly:
	while (!(limSws[RIGHT_LSW]->read())) {
		Plotter::plotLine(0, 0, 1, 0, 2);
		xPos--;
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
	ySteps = stepCount;	// should this be stepCount+1 ?!
	// set the current YPos:
	yPos = 0;


	// Drive downwards until the upper limit switch opens:
	// increase YPos accordingly:
	while (!(limSws[UP_LSW]->read())) {
		Plotter::plotLine(0, 0, 0, 1, 2);
		yPos++;
	}

	// drive to the center of the canvas:
	Plotter::plotLine(xPos, yPos, xSteps/2, ySteps/2, 2);
}
