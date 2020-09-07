#include "Parser.h"

<<<<<<< HEAD
#include "FileHandler.h"
#include "MockPipe.h"
#include "ParsedGdata.h"

int main()
{
	FileHandler pipe("gcode01.txt");
	//MockPipe pipe;
	
	const int maxLineLen = 50;
	char gCodeStr[maxLineLen];
	std::string gCodeLine;

	std::regex g1("G1 X[-+]?[0-9]*[.][0-9]+ Y[-+]?[0-9]*[.][0-9]+ A[0-1]");
	std::regex g28("G28");
	std::regex m1("M1 ([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
	std::regex m2("M2 [U]([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5]) [D]([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
	std::regex m4("M4 ([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
	std::regex m5("M5 A0 B0 H310 W380 S80");
	std::regex m10("M10");
	std::regex m11("M11");

	if (pipe.init()) {

		while (pipe.getLine(gCodeStr)) { //reads the file line by line
			gCodeLine.assign(gCodeStr);

			// The place to extract all the parsed data:
			// It's not used for anything, yet!
			ParsedGdata data;

			std::printf("%s\r\n", gCodeStr);

			if (gCodeLine.length() >= 3) {
				if (gCodeLine[0] == 'G') {
					if (gCodeLine[1] == '1') {
						if (std::regex_match(gCodeLine, g1)) {
							pipe.sendAck();

							data.codeType = GcodeType::G1;
							data.PenXY.X = std::stof(gCodeLine.substr(gCodeLine.find("X") + 1, (gCodeLine.substr(gCodeLine.find("X") + 1)).find(" ")));
							data.PenXY.Y = std::stof(gCodeLine.substr(gCodeLine.find("Y") + 1, (gCodeLine.substr(gCodeLine.find("Y") + 1)).find(" ")));
						}
						else {
							pipe.sendErr();
						}
					}
					else if (gCodeLine[1] == '2' && gCodeLine[2] == '8') {
						if (std::regex_match(gCodeLine, g28)) {
							pipe.sendAck();

							data.codeType = GcodeType::G28;
						}
						else {
							pipe.sendErr();
						}
					}
				}
				else if (gCodeLine[0] == 'M') {

					if (gCodeLine[1] == '1' && gCodeLine[2] == '0') {
						if (std::regex_match(gCodeLine, m10)) {
							pipe.sendAck();

							data.codeType = GcodeType::M10;
						}
						else {
							pipe.sendErr();
						}
					}
					else if (gCodeLine[1] == '1' && gCodeLine[2] == '1' && gCodeLine.length() == 2) {
						pipe.sendAck();

						data.codeType = GcodeType::M11;
					}
					else if (gCodeLine[1] == '1') {
						if (std::regex_match(gCodeLine, m1)) {
							pipe.sendAck();

							data.codeType = GcodeType::M1;
							data.penPosition = std::stoi(gCodeLine.substr(gCodeLine.find(" ") + 1));
						}
						else {
							pipe.sendErr();
						}
					}
				}
				else if (gCodeLine[1] == '2') {
					if (std::regex_match(gCodeLine, m2)) {
						pipe.sendAck();

						data.codeType = GcodeType::M2;
						data.penUp = stoi(gCodeLine.substr(gCodeLine.find(" ") + 1));
						data.penDown = stoi(gCodeLine.substr(gCodeLine.find("D")));
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '4') {
					if (std::regex_match(gCodeLine, m4)) {
						pipe.sendAck();

						data.codeType = GcodeType::M4;
						data.laserPower = std::stoi(gCodeLine.substr(gCodeLine.find(" ") + 1));
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '5') {
					if (std::regex_match(gCodeLine, m5)) {
						pipe.sendAck();

						data.codeType = GcodeType::M5;
					}
					else pipe.sendErr();
				}
				else {
					pipe.sendErr();
				}

=======
Parser::Parser(GcodePipe* pipePtr_) : g1("G1 X[-+]?[0-9]*[.][0-9]+ Y[-+]?[0-9]*[.][0-9]+ A[0-1]"), g28("G28"), m1("M1 ([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"),
m2("M2 [U]([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5]) [D]([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"),
m4("M4 ([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"), m5("M5 A0 B0 H310 W380 S80"), m10("M10"), m11("M11")
{
	pipePtr = pipePtr_;
	gCodeStr[0] = '\0';
}

bool Parser::init() {
	if (pipePtr->init())
		return true;
	else {
		printf("Gcode fetcher not ready.\n");
		return false;
	}
}

bool Parser::parseLine(ParsedGdata* data)
{
	pipePtr->getLine(gCodeStr);
	gCodeLine.assign(gCodeStr);
	printf("%s\r\n", gCodeStr);

	if (gCodeLine[0] == 'G') {
		if (gCodeLine[1] == '1') {
			if (std::regex_match(gCodeLine, g1)) {
				data->codeType = GcodeType::G1;
				data->PenXY.X = strtof(gCodeLine.substr(gCodeLine.find("X") + 1, (gCodeLine.substr(gCodeLine.find("X") + 1)).find(" ")).c_str(), nullptr);
				data->PenXY.Y = strtof(gCodeLine.substr(gCodeLine.find("Y") + 1, (gCodeLine.substr(gCodeLine.find("Y") + 1)).find(" ")).c_str(), nullptr);
				std::printf("coords to go to = X:%f Y:%f\r\n", data->PenXY.X, data->PenXY.Y);
				pipePtr->sendAck(); //TODO: these should probably be in a different function
				return true;
			}
			else {
				pipePtr->sendErr();
				return false;
			}
		}
		else if (gCodeLine[1] == '2' && gCodeLine[2] == '8') {
			if (std::regex_match(gCodeLine, g28)) { //TODO: what do we put in the structure here?
				data->codeType = GcodeType::G28;
				pipePtr->sendAck();
				std::printf("going to origin\r\n");
				return true;
			}
			else {
				pipePtr->sendErr();
				return false;
			}
		}
	}
	else if (gCodeLine[0] == 'M') {
		if (gCodeLine[1] == '1' && gCodeLine[2] == '0') {
			if (std::regex_match(gCodeLine, m10)) {
				data->codeType = GcodeType::M10;
				char tmp[maxLineLen];
				sprintf(tmp, "M10 XY %d %d 0.00 0.00 A%d B%d H0 S%d U%d D%d\r\n", data->height, data->width, data->Adir, data->Bdir, data->speed, data->penUp, data->penDown);
				pipePtr->sendLine(tmp);
				pipePtr->sendAck();
				printf("COM-port opened\r\n");
				return true;
>>>>>>> Rasmus
			}
			else {
				pipePtr->sendErr();
				return false;
			}
<<<<<<< HEAD
=======
		}

		else if (gCodeLine[1] == '1' && gCodeLine[2] == '1') {
			if (std::regex_match(gCodeLine, m11)) {
				char tmp[maxLineLen];
				sprintf(tmp, "%d %d %d %d\r\n", data->limitSw[0], data->limitSw[1], data->limitSw[2], data->limitSw[3]);
				pipePtr->sendLine(tmp);
				pipePtr->sendAck();
				return true;
			}
			else
				return false;
>>>>>>> Rasmus
		}
		else if (gCodeLine[1] == '1') {
			if (std::regex_match(gCodeLine, m1)) {
				data->codeType = GcodeType::M1;
				data->penPosition = atoi(gCodeLine.substr(gCodeLine.find(" ") + 1).c_str());
				pipePtr->sendAck();
				printf("penposition = %d\r\n", data->penPosition);
				return true;
			}
			else {
				pipePtr->sendErr();
				return false;
			}
		}
		else if (gCodeLine[1] == '2') {
			if (std::regex_match(gCodeLine, m2)) {
				pipePtr->sendAck();
				data->codeType = GcodeType::M2;
				data->penUp = atoi(gCodeLine.substr(gCodeLine.find("U") + 1).c_str());
				data->penDown = atoi(gCodeLine.substr(gCodeLine.find("D") + 1).c_str());
				printf("penUp = %d ; penDown = %d\r\n", data->penUp, data->penDown);
				return true;
			}
			else {
				pipePtr->sendErr();
				return false;
			}
		}
		else if (gCodeLine[1] == '4') {
			if (std::regex_match(gCodeLine, m4)) {
				pipePtr->sendAck();
				data->codeType = GcodeType::M4;
				data->laserPower = atoi(gCodeLine.substr(gCodeLine.find(" ") + 1).c_str());
				printf("penpower = %d\r\n", data->laserPower);
				return true;
			}
			else {
				pipePtr->sendErr();
				return false;
			}
		}
		else if (gCodeLine[1] == '5') {
			if (std::regex_match(gCodeLine, m5)) {
				data->codeType = GcodeType::M5;
				data->Adir = atoi(gCodeLine.substr(gCodeLine.find("A") + 1).c_str());
				data->Bdir = atoi(gCodeLine.substr(gCodeLine.find("B") + 1).c_str());
				data->height = atoi(gCodeLine.substr(gCodeLine.find("H") + 1).c_str());
				data->width = atoi(gCodeLine.substr(gCodeLine.find("W") + 1).c_str());
				data->speed = atoi(gCodeLine.substr(gCodeLine.find("S") + 1).c_str());
				pipePtr->sendAck();
				std::printf("plotter settings set");
				return true;
			}
			else pipePtr->sendErr();
			return false;
		}

	}

	else {
		pipePtr->sendErr();
		return false;
	}




}

Parser::~Parser() {

}