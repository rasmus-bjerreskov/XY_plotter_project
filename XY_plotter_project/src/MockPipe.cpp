#include "MockPipe.h"

MockPipe::MockPipe() {
	snprintf(gCodeLines[0], maxLineLen, "G1 X-0.50 Y101.50 A0");
	snprintf(gCodeLines[1], maxLineLen, "M");
	snprintf(gCodeLines[2], maxLineLen, "M18");
	snprintf(gCodeLines[3], maxLineLen, "M1 160");
	snprintf(gCodeLines[4], maxLineLen, "M1 160 X90.00");
	currLine = 0;
}


MockPipe::~MockPipe() {

}

bool MockPipe::clean() {
	return true;
}

bool MockPipe::getLine(char* dest) {
	if (currLine < numOfLines - 1) {
		snprintf(dest, maxLineLen, gCodeLines[currLine]);
		currLine++;
		return true;
	}

	return false;
}

bool MockPipe::sendLine(const char* dest) {
	printf("Plotter replies: %s", dest);
	return true;
}

bool MockPipe::sendAck() {
	printf("[%s]: passed\n", gCodeLines[currLine]);
	return true;
}

bool MockPipe::sendErr() {
	printf("[%s]: not passed\n", gCodeLines[currLine]);
	return true;
}
