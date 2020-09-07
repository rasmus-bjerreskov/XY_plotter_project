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
	#define MAX_CODE_LINE_LEN 100
	char codeLine[MAX_CODE_LINE_LEN];
	Tokens tokens;
	GcodePipe *pipe;
	static const char tokenDelimChar = ' ';
	static const char lineEndChar = '\n';

	char *tokenize();
	char *nextToken(void);

	bool gParser(ParsedGdata *data, char *tokLine);
	bool mParser(ParsedGdata *data, char *tokLine);
	bool savePenUDPosParser(ParsedGdata *data, char *tokLine);
	bool gotoOriginParser(ParsedGdata *data, char *tokLine);
	bool comOpenParser(ParsedGdata *data, char *tokLine);
	bool limitSwQueryParser(ParsedGdata *data, char *tokLine);

	bool validateFloatStr(char *FloatStr);

public:
	Parser(GcodePipe *fetcher);
	virtual ~Parser();

	bool parse(ParsedGdata *_data);
};

#endif /* PARSER_H_ */
