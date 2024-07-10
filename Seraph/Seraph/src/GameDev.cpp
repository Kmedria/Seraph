#include "GameDev.h"

GameDev::GameDev() {
	graphics = Graphics();
}

GameDev::~GameDev() {}

int GameDev::init() {
	graphics.initGraphics();
	return 0;
}

int GameDev::run() {
	bool running = true;
	
	// get initial data.
	// set world data
	// draw initial graphics

	while (graphics.isWindowOpen())
	{
		// Recieve controller/keyboard input

		// update enemy data
		// update terrain data

		// draw frame data
		//
		// graphics.draw();

		glfwPollEvents();
	}
	return 0;
}

int GameDev::cleanup() {
	graphics.cleanup();
	return 0;
}
