<<<<<<< HEAD
#pragma once

enum GcodeType { M1, M2, M4, M5, M10, M11, G1, G28 };
=======

#pragma once

enum class GcodeType { M1, M2, M4, M5, M10, M11, G1, G28 };
>>>>>>> Rasmus

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
<<<<<<< HEAD
	int speed;
=======
	int height;
	int width;
	int speed;
	int Adir;
	int Bdir;
>>>>>>> Rasmus
	int penUp;
	int penDown;
	int penPosition;
	int laserPower;
<<<<<<< HEAD
} ParsedGdata;

=======
	int limitSw[4];
} ParsedGdata;
>>>>>>> Rasmus
