#ifndef _ParsedGdata_h_
#define _ParsedGdata_h_

#include "CanvasCoordinates.h"

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

enum class RelModes { ABS, REL };

enum class GcodeType { M1, M2, M4, M5, M10, M11, G1, G28 };
#define MAX_STR_LEN 50

struct PlotInstruct_t{
	CanvasCoordinates_t newPos;
	GcodeType code;
	int penPos;
	RelModes relMode;
	uint32_t cnt;
};

struct ParsedGdata_t
{
	GcodeType codeType;
	CanvasCoordinates_t PenXY;
	CanvasCoordinates_t canvasLimits;
	int penUp;
	int penDown;
	int penCur;
	RelModes relativityMode;
	int speed;
	int Adir;
	int Bdir;
	int laserPower;
    int limitSw[4];
};

#endif
