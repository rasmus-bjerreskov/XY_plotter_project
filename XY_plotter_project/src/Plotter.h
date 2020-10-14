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

class Plotter {
public:
	Plotter();
	virtual ~Plotter();
	void plotLine(int x0_l, int y0_l, int x1_l, int y1_l, int us);
	void calibrateCanvas();

	void switchOffturn();
	bool getOffturn();

	int x0, y0, y1, x1, dx, dy, D, i;
	int prim1, prim2, prim3;
	DigitalIoPin *primaryIo;
	DigitalIoPin *secondaryIo;
	SemaphoreHandle_t sbRIT;

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

private:
	bool offturn;
};

#endif /* PLOTTER_H_ */
