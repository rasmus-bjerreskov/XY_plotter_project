/*
 * Plotter.h
 *
 *  Created on: 14.10.2020
 *      Author: Santeri Rauhala
 */

#ifndef PLOTTER_H_
#define PLOTTER_H_

class Plotter {
public:
	Plotter();
	virtual ~Plotter();
	void plotLine(int x0_l, int y0_l, int x1_l, int y1_l, int us);

	void switchOffturn();
	bool getOffturn();

private:
	bool offturn;
};

#endif /* PLOTTER_H_ */
