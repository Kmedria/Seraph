#include "Controller.h"

Controller::Controller() {}
Controller::~Controller() {}

int Controller::init() {
	return 0;
}

InputData Controller::getInput() {
	return InputData(0, 0, 0, 0, 0);
}

int Controller::cleanup() {
	return 0;
}

InputData::InputData(int x, int y, int z, int yRot, int zRot) {
	s_x = x;
	s_y = y;
	s_z = z;

	s_zRot = zRot;
	s_yRot = yRot;
}

InputData::InputData() {
	s_x = 0;
	s_y = 0;
	s_z = 0;

	s_zRot = 0;
	s_yRot = 0;
}
