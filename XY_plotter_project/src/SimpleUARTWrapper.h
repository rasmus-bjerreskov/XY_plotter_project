/*
 * SimpleUARTWrapper.h
 *
 *  Created on: 7 Sep 2020
 *      Author: Rasmus
 */




#ifndef SIMPLEUARTWRAPPER_H_
#define SIMPLEUARTWRAPPER_H_

#include "GcodePipe.h"
#include "FreeRTOS.h"
#include "semphr.h"

class SimpleUART_Wrapper: public GcodePipe {
public:
	SimpleUART_Wrapper(SemaphoreHandle_t mutex);
	bool init();
	bool clean();
	bool getLine(char *dest);
	bool sendLine(const char *dest);
	bool sendAck();
	bool sendErr();
	virtual ~SimpleUART_Wrapper();

private:
	SemaphoreHandle_t mutex;
};

#endif /* SIMPLEUARTWRAPPER_H_ */
