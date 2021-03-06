#ifndef CANVASCOORDINATES_H_
#define CANVASCOORDINATES_H_

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

const int UMS_PER_STEP = 25;

struct CanvasCoordinates_t
{
    union {
        int Xum;    // the X coordinate position in micrometers
        int Xmm;    // the X coordinate position in millimeters
    };

    union {
        int Yum;    // the Y coordinate position in micrometers
        int Ymm;    // the Y coordinate position in millimeters
    };

    int Xsteps;        // the X coordinate position in steps
    int Ysteps;        // the Y coordinate position in steps
};

#endif /* CANVASCOORDINATES_H_ */
