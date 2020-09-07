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

class Parser {
private:
	char codeLine[100];
	GcodePipe pipe;
	static const char *tokenDelimStr;
	static const char* lineEndStr;

	bool gParser(ParsedGdata *data, char *tokLine);
	bool mParser(ParsedGdata *data, char *tokLine);
	bool savePenUDPosParser(ParsedGdata *data, char *tokLine);
	bool gotoOriginParser(ParsedGdata *data, char *tokLine);

	bool validateFloatStr(char *FloatStr);

public:
	Parser(GcodePipe *fetcher);
	virtual ~Parser();

	bool parse(ParsedGdata *_data);
};

#endif /* PARSER_H_ */
