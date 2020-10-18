#ifndef CANVASCOORDINATES_H_
#define CANVASCOORDINATES_H_

const int MM_SCALE_FACTOR = 100000;
const int SCALED_MMS_PER_STEP = 2500;

struct CanvasCoordinates_t
{
	int Xum;		// the X coordinate position in micrometers
	int Yum;		// the Y coordinate position in micrometers

	int Xsteps;		// the X coordinate position in steps
	int Ysteps;		// the Y coordinate position in steps
};

struct CanvasSize_t
{
	int Xmm;
	int Ymm;

	int Xsteps;
	int Ysteps;
};

#endif /* CANVASCOORDINATES_H_ */
