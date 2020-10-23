/*
 * PenServo.cpp
 *
 *  Created on: 8. okt. 2020
 *      Author: Rasmus
 */

#include "PenServoCtrl.h"

PenServoController::PenServoController(ParsedGdata_t *_data) {
	data = _data;

	/*SCT setup
	 * cf n refers to table in lpc15xx user manual
	 cf table 201 p 234 of user manual*/
	Chip_SCT_Init(LPC_SCTLARGE0);
	LPC_SCTLARGE0->CONFIG |= SCT_CONFIG_AUTOLIMIT_L; // two 16-bit timers, auto limit - EVENT[0] will clear timer cf 202
	LPC_SCTLARGE0->CTRL_L |= ((Chip_Clock_GetSystemClockRate() / 1000000 - 1)
			<< 5); // set prescaler, SCTimer/PWM clock = 1 MHz cf 203 p 240
	LPC_SCTLARGE0->MATCHREL[0].L = 20000 - 1; // match 0 @ 20000/1MHz = 20ms period
	LPC_SCTLARGE0->MATCHREL[1].L = convertPos(data->penCur); //non-inverted. duty 1000 = 1ms etc
	LPC_SCTLARGE0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only

	Chip_SWM_MovablePinAssign(SWM_SCT0_OUT0_O, 10); //assign servo motor as output cf 114

	LPC_SCTLARGE0->OUT[0].SET = (1 << 0); // event 0 will set SCT0_OUT0 - cf 231
	LPC_SCTLARGE0->OUT[0].CLR = (1 << 1); // event 1 will clear SCT0_OUT0 - cf 230
	LPC_SCTLARGE0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg

}

void PenServoController::updatePos(int newVal) {
	if ((newVal == data->penDown || newVal == data->penUp) && newVal != curVal) {
		LPC_SCTLARGE0->MATCHREL[1].L = convertPos(newVal);
		curVal = newVal;
	}
}

//convert value 0-255 to 1000-2000
uint16_t PenServoController::convertPos(int val) {

	uint16_t tmp = SERVO_MIN + (float) val / 255 * 1000;
	uint16_t newVal;
	tmp > SERVO_MAX ? newVal = SERVO_MAX : newVal = tmp;

	return newVal;
}

PenServoController::~PenServoController() {
	// TODO Auto-generated destructor stub
}

