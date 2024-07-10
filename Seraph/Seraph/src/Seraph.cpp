#include "GameDev.h"
#include <iostream>

int main() {
    GameDev gameDevSession = GameDev();
    int state = gameDevSession.init();
    if (state != 0) {
        std::cout << "Initialization Failed";
    } else {
        state = gameDevSession.run();
        if (state != 0) {
            std::cout << "Failed during run process";
        }
        else {
            return gameDevSession.cleanup();
        }
    }

    return -1;

}