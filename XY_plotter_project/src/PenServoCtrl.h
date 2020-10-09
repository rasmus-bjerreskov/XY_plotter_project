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
	PenServoController(int up, int down);
	int getCurPos();

	void operator=(const int);
	virtual ~PenServoController();

	int up;
	int down;
private:
	int curPos;
	void set_pos(int);

	static const int SERVO_MIN = 1000;
	static const int SERVO_MAX = 2000;
};

#endif /* PENSERVO_H_ */
