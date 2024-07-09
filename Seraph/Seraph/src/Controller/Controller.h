#pragma once
#include "Graphics/Graphics.h"

struct InputData {
	int s_x = 0;
	int s_y = 0;
	int s_z = 0;

	int s_yRot;
	int s_zRot;

	InputData();
	InputData(int x, int y, int z, int yRot, int zRot);
};

class Controller {
public:
	Controller();
	~Controller();

	int init();
	InputData getInput();
	int cleanup();
};