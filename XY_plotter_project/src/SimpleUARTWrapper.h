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
	bool getLine(char *destination_string);
	bool getLine(char *destination_string, int buffer_size);
	bool sendLine(const char *destination_string);
	bool sendAck();
	bool sendErr();
	virtual ~SimpleUART_Wrapper();

private:
	SemaphoreHandle_t mutex;
	static const int MAX_BUF_LEN = 80;
};

#endif /* SIMPLEUARTWRAPPER_H_ */
