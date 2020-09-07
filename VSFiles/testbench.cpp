#include "Parser.h"
#include "ParsedGdata.h"
#include "GcodePipe.h"
#include "MockPipe.h"
int main() {
	ParsedGdata data;
	data.codeType = GcodeType::M1;
	data.PenXY = { 0,0 };
	data.UpperRightLimit = { 0,0 };
	data.LowerLeftLimit = { 0,0 };
	data.height = 380;
	data.width = 310;
	data.speed = 80;
	data.Adir = 0;
	data.Bdir = 0;
	data.penUp = 160;
	data.penDown = 90;
	data.penPosition = 160;
	data.laserPower = 0;
	for (int i = 0; i < 4; i++) {
		data.limitSw[i] = 1;
	}

	MockPipe pipe;
	Parser parser(&pipe);
	for (int i = 0; i < 10; i++){
		parser.parseLine(&data);
	}
}