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

#include "ParsedGdata.h"

class PenServo {
public:
	PenServo(ParsedGdata_t* data, LPC_SCT_T* pSCT);
	void update_pos();
	void pen_down();
	void pen_up();
	virtual ~PenServo();

private:
	void set_pos(int);

	ParsedGdata_t* plot_data;
	LPC_SCT_T* pSCT;
	static const int SERVO_MIN = 1000;
	static const int SERVO_MAX = 2000;
};

#endif /* PENSERVO_H_ */
