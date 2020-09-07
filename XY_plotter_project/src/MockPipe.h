#pragma once

#include <string>

#include "GcodePipe.h"

class MockPipe : public GcodePipe {
private:
	static const int numOfLines = 5;
	static const int maxLineLen = 50;
	char gCodeLines[numOfLines][maxLineLen];
	int currLine;

	int maxLines;

public:
	MockPipe();
	MockPipe(int _maxLines);
	virtual ~MockPipe();
	virtual bool init();
	virtual bool clean();
	virtual bool getLine(char* dest);
	virtual bool sendLine(const char* dest);
	virtual bool sendAck();
	virtual bool sendErr();
};

