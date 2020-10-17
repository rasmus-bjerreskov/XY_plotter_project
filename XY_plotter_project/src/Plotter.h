/*
 * Plotter.h
 *
 *  Created on: 14.10.2020
 *      Author: Santeri Rauhala
 */

#ifndef PLOTTER_H_
#define PLOTTER_H_

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif
#include "stdlib.h"
#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "CanvasCoordinates.h"

class Plotter {
public:
	Plotter();
	virtual ~Plotter();

	// used to actually draw the line. it uses penXYPos as the origin of the pen position in the canvas.
	// x1_l is the ending x position, y1_l is the ending y position , us is the speed of the plotter (1000000 / pps)
	void plotLine(int x1_l, int y1_l);

	void calibrateCanvas();			// used to determen which limit switch is which and also the size of the canvas and length of step

	void isr(portBASE_TYPE xHigherPriorityWoken); // used to make managing the IRQHandlerer bit easier

	void switchOffturn();			// used to switch "offturn"-variable between true and false
	bool getOffturn();				// used to get the value private "offturn"-variable

	int dx;							// difference between x1 and x0 =abs(x1-x0)
	int dy;							// difference between y1 and y0 =abs(y1-y0)
	int D;							// function f(x,y) = D = 2*dy - dx or 2*dx - dy; depending on dominant axis used to determine when to step only dominant axis and when both
	int start;							// counter used to run right amount of steps. When x is primary axis it starts from x0 and y0 when y is primary
	int dest;						// used in algorithm to plot. It is x1 when x is primary axis and y when y-axis is.
	int prim_delta;						// used in algorithm to plot. It is dy when x is primary axis and dx when y-axis is.
	int second_delta;						// used in algorithm to plot. It is dx when x is primary axis and xy when y-axis is.
	int prim_cartes;				//primary cartesian direction
	int second_cartes;				//secondary cartesian direction

	DigitalIoPin *primaryIo;		// used to save dominant axis
	DigitalIoPin *secondaryIo;		// used to save non-dominant axis
	SemaphoreHandle_t sbRIT;		// used to end rit-timer at right time
	CanvasSize_t canvasSize;		// used to contain the dimensions of the canvas in steps an in mms
	CanvasCoordinates_t penXYPos;	// used to contain the current XY-position of the pen in the canvas.

	DigitalIoPin *LSWPin1;			// used to hold IO-pins of limit switch 1
	DigitalIoPin *LSWPin2;			// used to hold IO-pins of limit switch 2
	DigitalIoPin *LSWPin3;			// used to hold IO-pins of limit switch 3
	DigitalIoPin *LSWPin4;			// used to hold IO-pins of limit switch 4
	DigitalIoPin *Xstep;			// used to hold IO-pins of stepper motor of that goes in X-axis
	DigitalIoPin *Ystep;			// used to hold IO-pins of stepper motor of that goes in Y-axis
	DigitalIoPin *Xdir;				// used to hold IO-pins of direction of the X-axis stepper motor
	DigitalIoPin *Ydir;				// used to hold IO-pins of direction of the X-axis stepper motor

	enum LSWLables {UP_LSW=0, RIGHT_LSW=1, DOWN_LSW=2, LEFT_LSW=3};	// used in calibrateCanvas to keep track which limit switch is on which side
	DigitalIoPin *limSws[4];										// used to hold those limit switches

private:
	enum direction {left = 0, right = 1, down = 0, up = 1};
	bool offturn;					//used in the IRQ to keep track when to write 1 and when to write 0 to stepper motors
	bool calibrate = false;

	// used to actually draw the line, x0_l is the starting x-position, y0_l is the starting y position,
	// x1_l is the ending x position, y1_l is the ending y position , us is the speed of the plotter (1000000 / pps)
	void plotLine(int x0_l, int y0_l, int x1_l, int y1_l);
};

#endif /* PLOTTER_H_ */
