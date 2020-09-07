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

Parser::Parser(GcodePipe *_pipe) {
	pipe = _pipe;
	tokens.currToken = NULL;
	tokens.numTokens = 0;
	tokens.currTokenNum = 0;

}

Parser::~Parser() {
	// TODO Auto-generated destructor stub
}

/*
 * Tokenizes the codeLine string. The codeLine string will change permanently.
 * After the tokenization, the nextToken-function can be used to access
 * consequent tokens of the string.
 *
 * @return the first token in the tokenized codeLine
 */
char* Parser::tokenize() {
    tokens.numTokens = 0;
    tokens.currTokenNum = 0;
    tokens.currToken = codeLine;

    char* currPos = codeLine;


    while (*currPos != '\0') {
        if (*currPos == tokenDelimChar) {
            tokens.numTokens++;
            *currPos = '\0';
        }

        currPos++;
    }

    if (tokens.numTokens > 0){
        tokens.numTokens++;
        return tokens.currToken;
    }
    else {
        return NULL;
    }
}

/**
 * Is used to get to the next token in the tokenized codeLine string.
 * Note: the next codeLine to be parsed has to be fetched first,
 * and tokenize has to be called before using nextToken!
 *
 * @return pointer to the next token if there are more tokens, NULL otherwise.
 */
char* Parser::nextToken(void) {
    char* currToken = tokens.currToken;

    if (tokens.currTokenNum == tokens.numTokens - 1) {
        return NULL;
    }
    else {
        while (*currToken != '\0') currToken++;

        currToken++;
        tokens.currTokenNum++;
        tokens.currToken = currToken;
        return currToken;
    }
}

/**
 * The main interface function for parsing gCodeLines
 * in the pipe. Waits until there's somethin to parse,
 * and then parses it.
 *
 * The parser follows the Context Free Grammar defined in
 * the file "CFG for Gcode" exactly.
 *
 * @param data Pointer to the parsed data
 * @return true if parsing was successful, false if not.
 */
bool Parser::parse(ParsedGdata *data) {
	while (!pipe->getLine(codeLine)) ;

	char *tokLine = tokenize();

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
		return true;
	}
	else {
		pipe->sendErr();
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
		return savePenUDPosParser(data, nextToken());
	else if (strcmp(tokLine+1, "28\n"))
		return gotoOriginParser(data, nextToken());
	else
		return false;
}

/**
 * The parser for M-code lines: "M1 ...", "M2 ..." and so on
 *
 *NOT FINISHED YET: currently only parses M10 and M11
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::mParser(ParsedGdata *data, char *tokLine) {
	if (strcmp(tokLine+1, "10\n"))
		return comOpenParser(data, nextToken());
	else if(strcmp(tokLine+1, "11\n"))
		return limitSwQueryParser(data, nextToken());
	else
		return true; // INCOMPLETE!!!
}

/**
 * The parser for "G1 ...": GOTO-POSITION -command
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

	tokLine = nextToken();

	// Parse the second token: "Y123.456"
	if (tokLine == NULL || tokLine[0] != 'Y')
		return false;

	if (!validateFloatStr(&tokLine[1]))
		return false;

	data->PenXY.X = strtof(tokLine+1, NULL);

	tokLine = nextToken();

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
	if (tokLine[2] != lineEndChar)
		return false;

	tokLine = nextToken();

	if (tokLine != NULL)
		return false;

	// Finally, add the code type to data:
	data->codeType = GcodeType::G1;
	return true;
}

/**
 * The parser for "G28\n" GOTO-ORIGIN -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::gotoOriginParser(ParsedGdata *data, char *tokLine) {
	if (tokLine == NULL) {
		data->codeType = GcodeType::G28;
		return true;
	}
	else {
		return false;
	}
}

/**
 * The parser for "M10\n" COMOPEN -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::comOpenParser(ParsedGdata *data, char *tokLine) {
	if (tokLine == NULL) {
		data->codeType = GcodeType::M10;
		return true;
	}
		else
			return true;
}

/**
 * The parser for "M11\n" LIMIT-SW-QUERY -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::limitSwQueryParser(ParsedGdata *data, char *tokLine) {
	if (tokLine == NULL) {
		data->codeType = GcodeType::M11;
		return true;
	}
	else
		return false;
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
