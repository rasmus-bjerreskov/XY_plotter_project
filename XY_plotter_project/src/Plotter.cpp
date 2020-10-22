/*
 * Plotter.cpp
 *
 *  Created on: 14.10.2020
 *      Author: Santeri Rauhala
 */

#include "Plotter.h"
#include "ITM_write.h"
#include <stdlib.h>

#define DEBUG

static Plotter *plotter; // used to hold the only plotter

// in constructor the IO-pins and their settings are set, also some starting values
Plotter::Plotter() {

	plotter = this;

	offturn = false;

	LSWPin1 = new DigitalIoPin(1, 3, DigitalIoPin::pullup);
	LSWPin2 = new DigitalIoPin(0, 0, DigitalIoPin::pullup);
	LSWPin3 = new DigitalIoPin(0, 9, DigitalIoPin::pullup);
	LSWPin4 = new DigitalIoPin(0, 29, DigitalIoPin::pullup);
	Xstep = new DigitalIoPin(0, 24, DigitalIoPin::output, true);
	Ystep = new DigitalIoPin(0, 27, DigitalIoPin::output, true);
	XdirCtrl = new DigitalIoPin(1, 0, DigitalIoPin::output, true);
	YdirCtrl = new DigitalIoPin(0, 28, DigitalIoPin::output, true);

	sbRIT = xSemaphoreCreateBinary();

}

Plotter::~Plotter() {
	// TODO Auto-generated destructor stub
}

//move plotter head to relative coordinates
void Plotter::plotLine(int x1_l, int y1_l) {
	plotLine(penXYPos.Xsteps, penXYPos.Ysteps, x1_l, y1_l);
}

//move plotter head to absolute coordinates
void Plotter::plotLine(int x0, int y0, int x1, int y1) {
	//stepper frequency - one step for every other pulse
	int pps;
#ifdef DEBUG
	if (calibrate)
		pps = 2 * 2000;
	else
		pps = 2 * 800;
#else
	pps = 2 * 800;
#endif

	if (calibrate) { //we don't know our exact position during calibration
		penXYPos.Xsteps = 1;
		penXYPos.Ysteps = 1;
	}
	// used to determine the frequency of the steps
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() / pps;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);

	//set stepper directions and limit switches to monitor - 0,0 is lower left corner
	int x_dir; 	//plotter direction for the axes
	int y_dir;
	DigitalIoPin *x_lsw;
	DigitalIoPin *y_lsw;
	if (x1 > x0) {
		XdirCtrl->write(right);
		x_dir = 1;
		x_lsw = limSws[RIGHT_LSW];
	} else {
		XdirCtrl->write(left);
		x_dir = -1;
		x_lsw = limSws[LEFT_LSW];
	}

	if (y1 > y0) {
		YdirCtrl->write(up);
		y_dir = 1;
		y_lsw = limSws[UP_LSW];
	} else {
		YdirCtrl->write(down);
		y_dir = -1;
		y_lsw = limSws[DOWN_LSW];
	}

	dx = abs(x1 - x0);
	dy = abs(y1 - y0);

	//selecting correct octant, choosing whether x or y is the primary axis
	if (dx > dy) { // when x is dominant axis
		prim_cart = x_dir;
		sec_cart = y_dir;
		prim_loc = &penXYPos.Xsteps;
		sec_loc = &penXYPos.Ysteps;
		prim_lim = canvasSize.Xsteps;
		sec_lim = canvasSize.Ysteps;
		start = x0;
		dest = x1;
		prim_delta = dx;
		sec_delta = dy;
		primaryIo = Xstep;
		secondaryIo = Ystep;
		prim_limsw = x_lsw;
		sec_limsw = y_lsw;

		D = 2 * dy - dx;
	} else {	// when y is dominant axis
		prim_cart = y_dir;
		sec_cart = x_dir;
		prim_loc = &penXYPos.Ysteps;
		sec_loc = &penXYPos.Xsteps;
		prim_lim = canvasSize.Ysteps;
		sec_lim = canvasSize.Xsteps;
		start = y0;
		dest = y1;
		prim_delta = dy;
		sec_delta = dx;
		primaryIo = Ystep;
		secondaryIo = Xstep;
		prim_limsw = y_lsw;
		sec_limsw = x_lsw;

		D = 2 * dx - dy;
	}

	if (!calibrate) {
		char str[40];
		sprintf(str, "from x%d y%d to x%d y%d\n", x0, y0, x1, y1);
		ITM_write(str);
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
	if (xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	} else {
		// unexpected error
	}
}

// using Bresenham's line algorithm to determine when to step with dominant axis only and when with both
void Plotter::isr(portBASE_TYPE xHigherPriorityWoken) {
	if (!offturn) { // when to write 1 on steppers
		if (start != dest) { // run as long as i = x0/y0 is smaller or equal to x1/y1
			offturn = !offturn;
			if (!calibrate) { //during calibration, limits are still undefined and have to be ignored

				*prim_loc += prim_cart; //this should still be updated to keep track of the intended location out of bounds
				if (*prim_loc <= prim_lim && *prim_loc >= 0
						&& prim_limsw->read()) {
					primaryIo->write(1);
				} else {
					xSemaphoreGiveFromISR(binPen, &xHigherPriorityWoken); //raise pen
				}

				//Bresenham's
				if (D > 0) {
					*sec_loc += sec_cart;
					if (*sec_loc <= sec_lim && *sec_loc >= 0
							&& sec_limsw->read()) {
						secondaryIo->write(1);
					} else {
						xSemaphoreGiveFromISR(binPen, &xHigherPriorityWoken); //raise pen
					}
					D = D + 2 * (sec_delta - prim_delta);
				} else {
					D = D + 2 * sec_delta;
				}
			} else {
				primaryIo->write(1);
			}
		} else {	// when i reaches x1/y1 disable the timer
			Chip_RIT_Disable(LPC_RITIMER); // disable timer

			// Give semaphore and set context switch flag if a higher priority task was woken up
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
		}
	}

	else { // when to write 0 on the steppers
		offturn = !offturn;
		Xstep->write(0);
		Ystep->write(0);
		start += prim_cart;

	}
}

// used to determen which limit switch is which and also the size of the canvas and length of step and go to starting position (0,0)
void Plotter::calibrateCanvas() {
	calibrate = true;
	int stepCount = 0;

	// calibrate XMotor:
	// Drive X-motor to left until a limit switch is hit:

	while (!LSWPin1->read() || !LSWPin2->read() || !LSWPin3->read()
			|| !LSWPin4->read()) {
		vTaskDelay(2); //waiting for all limit switches to be opened
	}

	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read()
			&& LSWPin4->read()) {
		Plotter::plotLine(1, 0, 0, 0);
	}

	// Record which limit switch was hit:
	limSws[LEFT_LSW] =
			(!LSWPin1->read()) ?
					LSWPin1 :
					((!LSWPin2->read()) ?
							LSWPin2 : ((!LSWPin3->read()) ? LSWPin3 : LSWPin4));
#ifdef DEBUG
	char str[40];
	sprintf(str, "limits: %d %d %d %d\n", LSWPin1->read(), LSWPin2->read(),
			LSWPin3->read(), LSWPin4->read());
	ITM_write(str);
	sprintf(str, "current limit: %d\n", limSws[LEFT_LSW]->read());
	ITM_write(str);
#endif
	// Drive to right until the left limit switch opens
	while (!(limSws[LEFT_LSW]->read())) {
		Plotter::plotLine(0, 0, 1, 0);
	}

	// Drive the X-motor to the right until another limit Switch is hit:
	// start counting steps
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read()
			&& LSWPin4->read()) {
		Plotter::plotLine(0, 0, 1, 0);
		stepCount++;
	}

	// Record which limit switch it was:
	limSws[RIGHT_LSW] =
			(!LSWPin1->read()) ?
					LSWPin1 :
					((!LSWPin2->read()) ?
							LSWPin2 : ((!LSWPin3->read()) ? LSWPin3 : LSWPin4));

	// Drive to left until limit switch opens
	while (!(limSws[RIGHT_LSW]->read())) {
		Plotter::plotLine(1, 0, 0, 0);
		stepCount--;
	}

	canvasSize.Xsteps = stepCount + 1; //"size" is beyond legal boundary, so it should be one larger
	// set the current XPos:
	penXYPos.Xsteps = canvasSize.Xsteps; // The left side is the zero coordinate

	// Reset the step count:
	stepCount = 0;

	// calibrate YMotor:
	// Drive Y-motor to down until a limit switch is hit:
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read()
			&& LSWPin4->read()) {
		Plotter::plotLine(0, 1, 0, 0);
	}

	// Record which limit switch was hit:
	limSws[DOWN_LSW] =
			(!LSWPin1->read()) ?
					LSWPin1 :
					((!LSWPin2->read()) ?
							LSWPin2 : ((!LSWPin3->read()) ? LSWPin3 : LSWPin4));

	// Drive up until the down limit switch opens:
	while (!(limSws[LSWLables::DOWN_LSW]->read())) {
		Plotter::plotLine(0, 0, 0, 1);
	}

	// Drive the Y-motor upwards until another limit Switch is hit:
	// count all the steps while driving
	while (LSWPin1->read() && LSWPin2->read() && LSWPin3->read()
			&& LSWPin4->read()) {
		Plotter::plotLine(0, 0, 0, 1);
		stepCount++;
	}

	// Record which limit switch it was:
	limSws[UP_LSW] =
			(!LSWPin1->read()) ?
					LSWPin1 :
					((!LSWPin2->read()) ?
							LSWPin2 : ((!LSWPin3->read()) ? LSWPin3 : LSWPin4));

	// set the current YPos:

	// Drive downwards until the upper limit switch opens:
	// increase YPos accordingly:
	while (!(limSws[UP_LSW]->read())) {
		Plotter::plotLine(0, 1, 0, 0);
		stepCount--;
	}
	canvasSize.Ysteps = stepCount + 1;
	penXYPos.Ysteps = stepCount;
	penXYPos.Xsteps = canvasSize.Xsteps - 1; // The left side is the zero coordinate
	calibrate = false;
	ITM_write("Calibration done\n");
	// drive to 0, 0
	Plotter::plotLine(0, 0);
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
