#include <iostream>
#include <string>
#include <regex>
#include <fstream>

#include "FileHandler.h"
#include "MockPipe.h"

Parser::Parser()
{
	MockPipe pipe;

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

	if (pipe.init()) {

		int times = 25;
		while (pipe.getLine(gCodeStr) && --times > 0) { //reads the file line by line
			gCodeLine.assign(gCodeStr);

			std::printf("%s\r\n", gCodeStr);

			if (gCodeLine[0] == 'G') {
				if (gCodeLine[1] == '1') {
					if (std::regex_match(gCodeLine, g1)) {
						pipe.sendAck();
						std::printf("coords to go to = X:%s Y:%s\r\n",
							gCodeLine.substr(gCodeLine.find("X") + 1, (gCodeLine.substr(gCodeLine.find("X") + 1)).find(" ")).c_str(),
							gCodeLine.substr(gCodeLine.find("Y") + 1, (gCodeLine.substr(gCodeLine.find("Y") + 1)).find(" ")).c_str());
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '2' && gCodeLine[2] == '8') {
					if (std::regex_match(gCodeLine, g28)) {
						pipe.sendAck();
						std::printf("going to origin\r\n");
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
						std::printf("COM-port opened\r\n");
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '1') {
					if (std::regex_match(gCodeLine, m1)) {
						pipe.sendAck();
						std::printf("penposition = %s\r\n", gCodeLine.substr(gCodeLine.find(" ") + 1).c_str());
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '2') {
					if (std::regex_match(gCodeLine, m2)) {
						pipe.sendAck();
						std::printf("penUp = %s ; penDown = %s\r\n", gCodeLine.substr(gCodeLine.find(" ") + 1).c_str(), gCodeLine.substr(gCodeLine.find("D")).c_str());
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '4') {
					if (std::regex_match(gCodeLine, m4)) {
						pipe.sendAck();
						std::printf("penpower = %s\r\n", gCodeLine.substr(gCodeLine.find(" ") + 1).c_str());
					}
					else {
						pipe.sendErr();
					}
				}
				else if (gCodeLine[1] == '5') {
					if (std::regex_match(gCodeLine, m5)) {
						pipe.sendAck();
						std::printf("plotter setting setted");
					}
					else pipe.sendErr();
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