#include <iostream>
#include <string>
#include <regex>
#include <fstream>

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

			}
			else {
				pipe.sendErr();
			}
		}
	}
	else {
		printf("Gcode fetcher not ready.\n");
	}

	std::cin;

	return 0;
}