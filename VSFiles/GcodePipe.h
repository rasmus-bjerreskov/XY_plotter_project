#pragma once

class GcodePipe {
public:
	virtual bool init() = 0;
	virtual bool clean() = 0;
	virtual bool getLine(char* dest) = 0;
	virtual bool sendLine(const char* dest) = 0;
	virtual bool sendAck() = 0;
	virtual bool sendErr() = 0;
	virtual ~GcodePipe() = 0;
};

