/*
 * PenServo.cpp
 *
 *  Created on: 8. okt. 2020
 *      Author: Rasmus
 */

#include "PenServo.h"

PenServo::PenServo(ParsedGdata_t* data, LPC_SCT_T* _pSCT) {
	plot_data = data;
	pSCT = _pSCT;

	/*SCT setup
	 * cf n refers to table in lpc15xx user manual
	cf table 201 p 234 of user manual*/
	Chip_SCT_Init (pSCT);
	pSCT->CONFIG |= SCT_CONFIG_AUTOLIMIT_L; // two 16-bit timers, auto limit - EVENT[0] will clear timer cf 202
	pSCT->CTRL_L |= ((Chip_Clock_GetSystemClockRate() / 1000000 - 1)
			<< 5); // set prescaler, SCTimer/PWM clock = 1 MHz cf 203 p 240
	pSCT->MATCHREL[0].L = 20000 - 1; // match 0 @ 20000/1MHz = 20ms period
	pSCT->MATCHREL[1].L = SERVO_MAX; //non-inverted. duty 1000 = 1ms etc
	pSCT->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	pSCT->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	pSCT->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	pSCT->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only

	Chip_SWM_MovablePinAssign(SWM_SCT0_OUT0_O, 10); //assign servo motor as output cf 114

	pSCT->OUT[0].SET = (1 << 0); // event 0 will set SCT0_OUT0 - cf 231
	pSCT->OUT[0].CLR = (1 << 1); // event 1 will clear SCT0_OUT0 - cf 230
	pSCT->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg

}

void PenServo::update_pos(){
	set_pos(plot_data->penPosition);
}

void PenServo::pen_down(){
	set_pos(plot_data->penDown);
}

void PenServo::pen_up(){
	set_pos(plot_data->penUp);
}

/*sets position based on input of 0-255*/
void PenServo::set_pos(int pos){
	if (pos > 255)
		pos = 255;
	else if (pos < 0)
		pos = 0;
	pSCT->MATCHREL[1].L = SERVO_MIN + (uint16_t)((float)pos / 255 * 1000);
}

PenServo::~PenServo() {
	// TODO Auto-generated destructor stub
}

