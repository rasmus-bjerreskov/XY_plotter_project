typedef enum {
	m1, m10, m11, m2, m4, g1, g28
} gCode_t

typdef struct gCodecontainer_t{ //maybe some of these fields should be in a different init struct, for the first m10 command
	gCode_t type;
	int xArea;
	int yArea;
	int aDir;
	int bDir;
	int speed;
	int penUpPos;
	int penDownPos;
	int laserPwr
	float xCoord;
	float yCoord;
	bool relCoords;
	int lmtSw[4];
} gCodecontainer_t;

int main() {

}
