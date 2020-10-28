/*
 * PenServo.h
 *
 *  Created on: 8. okt. 2020
 *      Author: Rasmus
 */

#ifndef PENSERVO_H_
#define PENSERVO_H_

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

class PenServoController {
public:
	PenServoController(int _downValue, int _upValue, int _curVal);
	void raisePen() { updatePos(upPos); }
	void lowerPen() { updatePos(downPos); }
	void updatePos(int newVal);
	void setPosVals(int down, int up) { downPos = down; upPos = up; }
	int getCurVal() { return curVal; }
	int getPenUpVal() { return upPos; }
	int getPenDownVal() {return downPos; }

	virtual ~PenServoController();

private:
	uint16_t convertPos(int val);
	int curVal;
	int upPos;
	int downPos;
	static const int SERVO_MIN = 1000;
	static const int SERVO_MAX = 2000;
};

#endif /* PENSERVO_H_ */
