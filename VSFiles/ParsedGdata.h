#pragma once

enum GcodeType { M1, M2, M4, M5, M10, M11, G1, G28 };

typedef struct Coordinates
{
	float X;
	float Y;
} Coordinates;

typedef struct ParsedGdata
{
	GcodeType codeType;
	Coordinates PenXY;
	Coordinates UpperRightLimit;
	Coordinates LowerLeftLimit;
	int speed;
	int penUp;
	int penDown;
	int penPosition;
	int laserPower;
} ParsedGdata;

