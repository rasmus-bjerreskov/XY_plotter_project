#ifndef CANVASCOORDINATES_H_
#define CANVASCOORDINATES_H_

const int MM_SCALE_FACTOR = 100000;
const int SCALED_MMS_PER_STEP = 2500;

struct CanvasCoordinates_t
{
	union {
		int Xum;	// the X coordinate position in micrometers
		int Xmm;	// the X coordinate position in millimeters
	};

	union {
		int Yum;	// the Y coordinate position in micrometers
		int Ymm;	// the Y coordinate position in millimeters
	};

	int Xsteps;		// the X coordinate position in steps
	int Ysteps;		// the Y coordinate position in steps
};

#endif /* CANVASCOORDINATES_H_ */
