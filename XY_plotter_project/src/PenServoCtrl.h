/*
 * PenServo.h
 *
 *  Created on: 8. okt. 2020
 *      Author: Rasmus
 */

#ifndef PENSERVO_H_
#define PENSERVO_H_

#include "ParsedGdata.h"

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

class PenServoController {
public:
	PenServoController(ParsedGdata_t *_data);
	void updatePos(int newVal);
	int getCurVal() { return curVal; }

	virtual ~PenServoController();

private:
	uint16_t convertPos(int val);
	ParsedGdata_t* data;
	int curVal;
	static const int SERVO_MIN = 1000;
	static const int SERVO_MAX = 2000;
};

#endif /* PENSERVO_H_ */
