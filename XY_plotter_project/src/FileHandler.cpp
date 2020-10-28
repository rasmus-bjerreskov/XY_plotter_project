/*
 * FileHandler.cpp
 *
 *  Created on: 2 Sep 2020
 *      Author: Rasmus
 */

#include "FileHandler.h"

FileHandler::FileHandler() : fh("gcode.txt"), fp(NULL), log_p(NULL) {

}

FileHandler::FileHandler(const char* _fh) : fh(_fh), fp(NULL), log_p(NULL) {

}

bool FileHandler::init() {
	log_p = fopen("g-code_log", "a");
	fp = fopen(fh, "r");
	if (fp != NULL)
		return true;
	return false;
}

bool FileHandler::clean() {
	fclose(fp);
	fclose(log_p);

	return true; //not much to check here
}

bool FileHandler::getLine(char* str) {
	if (fgets(str, 60, fp) != NULL) {
		strtok(str, "\n");
		fprintf(log_p, "%s\n", str);
		return true;
	}
	else {
		fprintf(log_p, "EOF\n");
		return false;
	}
}

bool FileHandler::sendLine(const char* dest) {
	fprintf(log_p, dest);
	return true;
}

bool FileHandler::sendAck() {
	fprintf(log_p, "OK\n");
	return true;
}

bool FileHandler::sendErr() {
	fprintf(log_p, "Error\n");
	return true;
}

FileHandler::~FileHandler() {
	// TODO Auto-generated destructor stub
}