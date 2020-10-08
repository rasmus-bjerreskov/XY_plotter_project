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
char *Parser::tokenize(){
	tokens.numTokens = 0;
	tokens.currTokenNum = 0;
	tokens.currToken = codeLine;

	char *currPos = codeLine;


	while(*currPos != '\0') {
		if(*currPos == tokenDelimChar) {
			tokens.numTokens++;
			*currPos = '\0';
		}

		currPos++;
	}

	return tokens.currToken;
}

/**
 * Is used to get to the next token in the tokenized codeLine string.
 * Note: the next codeLine to be parsed has to be fetched first,
 * and tokenize has to be called before using nextToken!
 *
 * @return pointer to the next token if there are more tokens, NULL otherwise.
 */
char *Parser::nextToken(void) {
	char *currToken = tokens.currToken;

	if (*currToken == tokens.numTokens-1) {
		return NULL;
	}
	else {
		while(*(currToken++) != '\0') ;

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
bool Parser::parse(ParsedGdata_t *data) {
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
			break;
	}

	if (success == true) {
		pipe->sendAck();
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
bool Parser::gParser(ParsedGdata_t *data, char *tokLine) {
	char *gCode = tokLine+1;
	if (strcmp(gCode, "1") == 0)
		return gotoPositionParser(data, nextToken());
	else if (strcmp(gCode, "28\n") == 0)
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
bool Parser::mParser(ParsedGdata_t *data, char *tokLine) {
	char *mCode = tokLine+1;

	if (strcmp(mCode, "10\n") == 0)
		return comOpenParser(data, nextToken());
	else if (strcmp(mCode, "11\n") == 0)
		return limitSwQueryParser(data, nextToken());
	else if (strcmp(mCode, "1") == 0)
		return setPenPosParser(data, nextToken());
	else if (strcmp(mCode, "2") == 0)
		return savePenUDPosParser(data, nextToken());
	else if (strcmp(mCode, "4") == 0)
		return setLaserPowParser(data, nextToken());
	else if (strcmp(mCode, "5") == 0)
		return saveStepperInfoParser(data, nextToken());
	else
		return false;
}

/**
 * The parser for "G1 ...": GOTO-POSITION -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::gotoPositionParser(ParsedGdata_t *data, char *tokLine) {
	// quick checking that the number of tokens mathces the G1 -code requirements:
	if (tokens.numTokens != 4)
		return false;

	// Parse the first token: "X123.456"
	if (tokLine[0] != 'X')
		return false;

	if (!validateFloatStr(&tokLine[1], false))
		return false;

	data->PenXY.X = atof(tokLine+1);

	tokLine = nextToken();

	// Parse the second token: "Y123.456"
	if (tokLine[0] != 'Y')
		return false;

	if (!validateFloatStr(&tokLine[1], false))
		return false;

	data->PenXY.X = atof(tokLine+1);

	tokLine = nextToken();

	// Parse the final token: "A0" or "A1":
	if (tokLine[0] != 'A')
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
bool Parser::gotoOriginParser(ParsedGdata_t *data, char *tokLine) {
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
bool Parser::comOpenParser(ParsedGdata_t *data, char *tokLine) {
	if (tokLine == NULL) {
		data->codeType = GcodeType::M10;
		return true;
	}
		else
			return false;
}

/**
 * The parser for "M11\n" LIMIT-SW-QUERY -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::limitSwQueryParser(ParsedGdata_t *data, char *tokLine) {
	if (tokLine == NULL) {
		data->codeType = GcodeType::M11;
		return true;
	}
	else
		return false;
}

/**
 * The parser for "M1 ..." SET-PEN-POS -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::setPenPosParser(ParsedGdata_t *data, char *tokLine) {
	// quick checking that the number of tokens mathces the M1 -code requirements:
	if (tokens.numTokens != 2)
		return false;

	if (!extract8BitUint(&data->penPosition, tokLine, true, lineEndChar))

	data->codeType = GcodeType::M1;
	return true;
}

/**
 * The parser for "M2 ..." SAVE-PEN-UD-POS -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::savePenUDPosParser(ParsedGdata_t *data, char *tokLine) {
	// quick checking that the number of tokens mathces the M2 -code requirements:
	if (tokens.numTokens != 3)
		return false;

	// extract the up value, if possible:
	if (tokLine[0] != 'U')
		return false;

	if (!extract8BitUint(&data->penUp, tokLine+1))
		return false;

	tokLine = nextToken();

	// extract the down value, if possible:
	if (tokLine[0] != 'D')
		return false;

	if (!extract8BitUint(&data->penDown, tokLine+1, true, lineEndChar))
		return false;

	data->codeType = GcodeType::M2;
	return true;
}

/**
 * The parser for "M4 ..." SET-LASER-POW -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::setLaserPowParser(ParsedGdata_t *data, char *tokLine) {
	// quick checking that the number of tokens mathces the M4 -code requirements:
	if (tokens.numTokens != 2)
		return false;

	// Extract the pen position value, if possible:
	if (!extract8BitUint(&data->laserPower, tokLine))
		return false;

	data->codeType = GcodeType::M4;
	return true;
}

/**
 * The parser for "M5 ..." SAVE-STEPPER-INFO -command
 *
 * @param data Pointer to the parsed data
 * @param tokLine The tokenized code line to be parsed
 * @return true if parsing was successful, false if it was not.
 */
bool Parser::saveStepperInfoParser(ParsedGdata_t *data, char *tokLine) {
	// M5 A0 B0 H310 W380 S80<LF>
	// quick checking that the number of tokens mathces the M5 -code requirements:
	if (tokens.numTokens != 6)
		return false;

	// Extract the X-axis direction:
	if (tokLine[0] != 'A')
		return false;

	if (!extractDirection(&data->Adir, tokLine+1))
		return false;


	// Extract the Y-axis direction:
	tokLine = nextToken();

	if (tokLine[0] != 'B')
		return false;

	if (!extractDirection(&data->Bdir, tokLine+1))
		return false;


	// Extract the height value:
	tokLine = nextToken();

	if (tokLine[0] != 'H')
		return false;

	if (!extractInt(&data->canvasLimits.Y, tokLine+1))
		return false;

	// Extract the width value:
	tokLine = nextToken();

	if (tokLine[0] != 'W')
		return false;

	if (!extractInt(&data->canvasLimits.X, tokLine+1))
		return false;

	// Extract the speed value
	tokLine = nextToken();

	if (tokLine[0] != 'S')
		return false;

	if (!extract8BitUint(&data->speed, tokLine+1, true, '\n'))
		return false;

	if (data->speed > 100)
		return false;

	data->codeType = GcodeType::M5;
	return true;
}

/**
 *  Extracts a direction of the form "0" or "1", with a possible delimiter character in the end.
 *  Checks that there are no other characters in the given string after the possible
 *  delimiter char.
 *
 *  @param storage the place to save the direction value
 *  @param dirStr the string representation of the direction value
 *  @param hasDelimChar true if dirStr contains exactly one trailing char after the
 *  	   direction value.
 *  @param delimChar if hasDelimChar is true, the trailing char in dirStr must exist and it must
 *  	   match the delimChar.
 *  @return true if the extraction succeeded, false otherwise
 */
bool Parser::extractDirection(int *storage, char *dirStr, bool hasDelimChar, char delimChar) {
	// Extract the direction:
		switch(*dirStr) {
			case '0':
				*storage = 0;
				break;
			case '1':
				*storage = 1;
				break;
			default:
				return false;
		}

		if (hasDelimChar) {
			return (dirStr[1] == delimChar && dirStr[2] == '\0');
		}
		else {
			return (dirStr[1] == '\0');
		}
}

/**
 *  Extracts an integer value from the given string, with a possible delimiter character in the end.
 *  Checks that there are no other characters in the given string after the possible delimiter char.
 *
 *  @param storage the place to save the integer value
 *  @param numStr the string representation of the int value
 *  @param hasDelimChar true if numStr contains exactly one trailing char after the
 *  	   direction value.
 *  @param delimChar if hasDelimChar is true, the trailing char in dirStr must exist and it must
 *  	   match the delimChar.
 *  @return true if the extraction succeeded, false otherwise
 */
bool extractInt(int *storage, char *numStr, bool hasDelimChar = false, char delimChar = '\0') {
	long val;
	char *rem;

	if (!isdigit(numStr[0]) || !(numStr[0] == '-' && isdigit(numStr[1])) )
		return false;

	// probable bug: if the given numLine is long enough ("1111111111..." etc.)
	// could strtol interpret it as a wrong value?
	val = strtol(numStr, &rem, 10);

	// probable bug in the type conversion:
	*storage = (int)val;

	if (hasDelimChar)
		return (rem[0] == delimChar && rem[1] == '\0');
	else
		return (rem[0] == '\0');
}

/**
 *  Extracts an 8 bit unsigned integer value from the given string, with a possible delimiter character in the end.
 *  Checks that there are no other characters in the given string after the possible delimiter char.
 *
 *  @param storage the place to save the 8 bit uint value
 *  @param numStr the string representation of the 8 bit unsigned int value
 *  @param hasDelimChar true if numStr contains exactly one trailing char after the
 *  	   direction value.
 *  @param delimChar if hasDelimChar is true, the trailing char in dirStr must exist and it must
 *  	   match the delimChar.
 *  @return true if the extraction succeeded, false otherwise
 */
bool Parser::extract8BitUint(int *storage, char *numStr, bool hasDelimChar, char delimChar) {
	long val;
	char *rem;

	if (!isdigit(*numStr))
		return false;

	// probable bug: if the given numLine is long enough ("1111111111..." etc.)
	// could strtol interpret it as a value between 0 and 255 in some rare conditions?
	// More research needed on strtol AND the long type.
	val = strtol(numStr, &rem, 10);
	if (val < 0 || val > 255)
		return false;

	*storage = (int)val;

	if (hasDelimChar)
		return (rem[0] == delimChar && rem[1] == '\0');
	else
		return (rem[0] == '\0');
}

/**
 * Validates, whether the given string is in a proper float form
 *
 * @param floatStr the string to be validated
 * @param hasDelimChar if this is true, stops when delimChar is met ignoring it, but
 * 					   checking that there are no more characters following.
 * @param delimChar the delimiter character
 * @return true if floatStr is of the form "123.456" or "123.456[DelimChar]" false otherwise
 */
bool Parser::validateFloatStr(char *floatStr, bool hasDelimChar, char delimChar) {
	bool failure = false;
	bool dotMetYet = false;
	bool delimMet = false;

	if (*floatStr == '-')
		floatStr++;

	while(*floatStr != '\0' && failure == false) {
		if(isdigit(*floatStr)) {
			floatStr++;
		}
		else if (*floatStr == '.' && dotMetYet == false) {
			floatStr++;
			dotMetYet = true;
		}
		else if (*floatStr == delimChar && hasDelimChar == true){
		    delimMet = true;
		    floatStr++;
			break;
		}
		else {
			failure = true;
			break;
		}
	}

	if (failure == true || !dotMetYet) {
		return false;
	}
	else if (hasDelimChar) {
	    if (!delimMet) {
	        return false;
	    }
	    else {
	        return (*floatStr == '\0');
	    }
	}
	else {
		return true;
	}
}
