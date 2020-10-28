/*
 * Parser.h
 *
 *  Created on: 5 Sep 2020
 *      Author: Rasmus
 */

#ifndef PARSER_H_
#define PARSER_H_

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <stdlib.h>

#include "GcodePipe.h"
#include "FileHandler.h"
#include "MockPipe.h"
#include "ParsedGdata.h"
//comment to force git to recognise changes
class Parser {
public:
	Parser(GcodePipe *);
	bool init();
	bool parseLine(ParsedGdata*); //takes pointer to data holding structure
	virtual ~Parser();

private:
	GcodePipe* pipePtr; //object pointer for receiving data string
	const std::regex g1;
	const std::regex g28;
	const std::regex m1;
	const std::regex m2;
	const std::regex m4;
	const std::regex m5;
	const std::regex m10;
	const std::regex m11;

	static const int maxLineLen = 50;
	char gCodeStr[maxLineLen];
	std::string gCodeLine;
};

#endif /* PARSER_H_ */
