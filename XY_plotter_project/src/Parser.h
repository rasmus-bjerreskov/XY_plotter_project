/*
 * Parser.h
 *
 *  Created on: 7.9.2020
 *      Author: rqqht
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "GcodePipe.h"
#include "ParsedGdata.h"

typedef struct Tokens {
	int numTokens;
	int currTokenNum;
	char *currToken;
} Tokens;

class Parser {
private:
	char codeLine[MAX_STR_LEN+1];
	Tokens tokens;
	static const char tokenDelimChar = ' ';
	static const char lineEndChar = '\n';

	char *tokenize();
	char *nextToken(void);

	bool gParser(ParsedGdata_t *data, char *tokLine);
	bool mParser(ParsedGdata_t *data, char *tokLine);
	bool gotoPositionParser(ParsedGdata_t *data, char *tokLine);
	bool gotoOriginParser(ParsedGdata_t *data, char *tokLine);
	bool comOpenParser(ParsedGdata_t *data, char *tokLine);
	bool limitSwQueryParser(ParsedGdata_t *data, char *tokLine);
	bool setPenPosParser(ParsedGdata_t *data, char *tokLine);
	bool savePenUDPosParser(ParsedGdata_t *data, char *tokLine);
	bool setLaserPowParser(ParsedGdata_t *data, char *tokLine);
	bool saveStepperInfoParser(ParsedGdata_t *data, char *tokLine);

	bool extractDirection(int *storage, char *tokLine, bool hasDelimChar = false, char delimChar = '\0');
	bool extractInt(int *storage, char *tokLine, bool hasDelimChar = false, char delimChar = '\0');
	bool extract8BitUint(int *storage, char *tokLine, bool hasDelimChar = false, char delimChar = '\0');

	bool validateFloatStr(char *FloatStr, bool hasDelimChar = false, char delimChar = '\0');

public:
	Parser();
	virtual ~Parser();

	bool parse(ParsedGdata_t *_data, char *line);
};

#endif /* PARSER_H_ */
