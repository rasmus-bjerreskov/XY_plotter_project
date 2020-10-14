#ifndef _ParsedGdata_h_
#define _ParsedGdata_h_

enum class GcodeType { M1, M2, M4, M5, M10, M11, G1, G28 };

struct Coordinates_t
{
	float X;
	float Y;
};

struct CanvasSize_t
{
	int X;
	int Y;
};


// Coordinates_t and CanvasSize could be merged into one union...

struct ParsedGdata_t
{
	GcodeType codeType;
	Coordinates_t PenXY;
	CanvasSize_t canvasLimits;
	int penUp;
	int penDown;
	int penCur;
	bool relativityMode;
	int speed;
	int Adir;
	int Bdir;
	int laserPower;
    int limitSw[4];
};

#endif
