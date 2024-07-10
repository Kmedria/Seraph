#pragma once
#include "Graphics/Graphics.h"

class GameDev {
public:

    GameDev();
    ~GameDev();

    int init();
    int run();
    int cleanup();

private:
    Graphics graphics;
};