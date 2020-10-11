/*
 * SimpleUARTWrapper.cpp
 *
 *  Created on: 7 Sep 2020
 *      Author: Rasmus
 */

#define debugprint

#include "SimpleUARTWrapper.h"
#include "FreeRTOS.h"
#include "semphr.h"
SimpleUART_Wrapper::SimpleUART_Wrapper(SemaphoreHandle_t mutex_) {
	mutex = mutex_;
}

bool SimpleUART_Wrapper::init() {
	return true;
}

bool SimpleUART_Wrapper::clean() {
	return true;
}

bool SimpleUART_Wrapper::getLine(char* str){
	return SimpleUART_Wrapper::getLine(str, MAX_BUF_LEN);
}


/* Receive text line from uart via board_uart
 Fills param str with characters from uart until LF or CR, which is stripped
 Returns false if buffer is empty at time of calling, otherwise true*/
bool SimpleUART_Wrapper::getLine(char *str, int n) { //TODO maybe n should be set just once on construction
	int c = 0;
	int buflen = (n <= MAX_BUF_LEN) ? n : MAX_BUF_LEN;
	int i = 0;

	xSemaphoreTake(mutex, portMAX_DELAY);
	c = Board_UARTGetChar();
	xSemaphoreGive(mutex);

	if (c == EOF)
		return false; //mdraw not yet ready

	while (c != '\r' && c != '\n' && i < (buflen - 1)) {
		if (c != EOF){
			str[i] = c;
			i++;
		}

		xSemaphoreTake(mutex, portMAX_DELAY);
		c = Board_UARTGetChar();
		xSemaphoreGive(mutex);
	}

	if (i < buflen - 1)
		str[i++] = '\n';

	str[i] = '\0';
#ifdef debugprint
	xSemaphoreTake(mutex, portMAX_DELAY);
	Board_UARTPutSTR("Debug: ");
	Board_UARTPutSTR(str);
	Board_UARTPutSTR("\r\n");
	xSemaphoreGive(mutex);
#endif
	return true;
}

bool SimpleUART_Wrapper::sendLine(const char *str) {
	xSemaphoreTake(mutex, portMAX_DELAY);
	Board_UARTPutSTR(str);
	xSemaphoreGive(mutex);
	return true;
}

bool SimpleUART_Wrapper::sendAck() {
	xSemaphoreTake(mutex, portMAX_DELAY);
	Board_UARTPutSTR("OK\r\n");
	xSemaphoreGive(mutex);
	return true;
}

bool SimpleUART_Wrapper::sendErr() {
	xSemaphoreTake(mutex, portMAX_DELAY);
	Board_UARTPutSTR("Error\r\n");
	xSemaphoreGive(mutex);
	return true;
}

SimpleUART_Wrapper::~SimpleUART_Wrapper() {
	// TODO Auto-generated destructor stub
}

