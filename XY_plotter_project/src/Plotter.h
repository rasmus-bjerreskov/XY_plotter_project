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
#include "DigitalIoPin.h"

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
	DigitalIoPin *primaryIo = NULL;
	DigitalIoPin *secondaryIo = NULL;

private:
	bool offturn;
};

#endif /* PLOTTER_H_ */
