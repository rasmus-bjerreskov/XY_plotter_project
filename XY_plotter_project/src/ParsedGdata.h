#ifndef _ParsedGdata_h_
#define _ParsedGdata_h_

enum class GcodeType { M1, M2, M4, M5, M10, M11, G1, G28 };

typedef struct Coordinates
{
	float X;
	float Y;
} Coordinates;

typedef struct CanvasSize
{
	int X;
	int Y;
} CanvasSize;

// Coordinates and CanvasSize could be merged into one union...

typedef struct ParsedGdata
{
	GcodeType codeType;
	Coordinates PenXY;
	CanvasSize canvasLimits;
	bool relativityMode;
	int speed;
	int Adir;
	int Bdir;
	int penUp;
	int penDown;
	int penPosition;
	int laserPower;
    int limitSw[4];
} ParsedGdata;

#endif
