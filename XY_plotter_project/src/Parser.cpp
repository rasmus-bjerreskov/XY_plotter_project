/*
 * Parser.cpp
 *
 *  Created on: 5 Sep 2020
 *      Author: Rasmus
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Parser.h"

const char *Parser::tokenDelimStr = " ";
const char *Parser::lineEndStr = "\n";

Parser::Parser(GcodePipe *_pipe) {
	pipe = _pipe;

}

Parser::~Parser() {
	// TODO Auto-generated destructor stub
}

/**
 * The main interface function for parsing gCodeLines
 * in the pipe. Waits until there's somethin to parse,
 * and then parses it
 *
 * @param data Pointer to the parsed data
 * @return true if parsing was successful, false if not.
 */
bool Parser::parse(ParsedGdata *data) {
	while (!pipe.getLine(codeLine)) ;

	char *tokLine = strtok(codeLine, " ");

	if (tokLine == NULL)
		return false;

	bool success = false;

	switch (tokLine[0]) {
		case 'G':
			success = gParser(data, tokLine);
			break;
		case 'M':
			success = mParser(data, tokLine);
			break;
		default:
			success = false;
			break;
	}

	if (success == true) {
		pipe.sendAck();
		return true;
	}
	else {
		pipe.sendErr();
		return false;
	}
}

/**
 * The parser for G-code lines: "G1 ..." and "G28 ..."
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::gParser(ParsedGdata *data, char *tokLine) {
	if (strcmp(tokLine+1, "1"))
		return savePenUDPosParser(data, strtok(NULL, tokLine));
	else if (strcmp(tokLine+1, "28"))
		return gotoOriginParser(data, strtok(NULL, tokLine));
	else
		return false;
}

/**
 * The parser for M-code lines: "M1 ..." and "M2 ..." and so on
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::mParser(ParsedGdata *data, char *tokLine) {
return true;
}

/**
 * The parser for "G1 ...": gotoPosition -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::savePenUDPosParser(ParsedGdata *data, char *tokLine) {
	// Parse the first token: "X123.456"
	if (tokLine == NULL || tokLine[0] != 'X')
		return false;

	if (!validateFloatStr(&tokLine[1]))
		return false;

	data->PenXY.X = strtof(tokLine+1, NULL);

	strtok(tokLine, NULL);

	// Parse the second token: "Y123.456"
	if (tokLine == NULL || tokLine[0] != 'Y')
		return false;

	if (!validateFloatStr(&tokLine[1]))
		return false;

	data->PenXY.X = strtof(tokLine+1, NULL);

	strtok(tokLine, NULL);

	// Parse the final token: "A0" or "A1":
	if (tokLine == NULL || tokLine[0] != 'A')
		return false;

	switch (tokLine[1]) {
		case '0':
			data->relativityMode = false;
			break;
		case '1':
			data->relativityMode = true;
			break;
		default:
			return false;
	}

	// finally, check that there's nothing more to be parsed:
	if (tokLine[2] != '\0')
		return false;

	strtok(tokLine, NULL);

	if (tokLine != NULL)
		return false;
	else
		return true;
}

/**
 * The parser for "G28 ..." gotoOrigin -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::gotoOriginParser(ParsedGdata *data, char *tokLine) {
	if (tokLine == NULL) {
		return false;
	}
	else if (strcmp(tokLine, lineEndStr)) {
		data->codeType = GcodeType::G28;
		return true;
	}
	else {
		return false;
	}
}

/**
 * Validates, whether the given string is in a proper float form
 *
 * @param floatStr the string to be validated
 * @return true if floatStr is of the form "12345.6789" false otherwise
 */
bool Parser::validateFloatStr(char *floatStr) {
	bool failure = false;
	bool dotMetYet = false;

	while(*floatStr != '\0' && failure == false) {
		if(isdigit(*floatStr)) {
			floatStr++;
		}
		else if (*floatStr == '.' && dotMetYet == false) {
			floatStr++;
		}
		else {
			failure = true;
		}
	}

	if (failure == true || !dotMetYet)
		return false;
	else
		return true;
}
