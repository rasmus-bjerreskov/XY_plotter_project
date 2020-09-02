#pragma once

/*
 * FileHandler.h
 *
 *  Created on: 2 Sep 2020
 *      Author: Rasmus
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#define _CRT_SECURE_NO_DEPRECATE

#include "GcodePipe.h"
//#include <fstream>
#include <cstdio>

class FileHandler : public GcodePipe {
public:
	FileHandler();
	FileHandler(const char*);
	bool init();
	bool clean();
	bool getLine(char*);
	bool sendAck();
	bool sendErr();
	virtual ~FileHandler();

private:
	const char* fh;
	FILE* fp;
	FILE* log_p;
};

#endif /* FILEHANDLER_H_ */

