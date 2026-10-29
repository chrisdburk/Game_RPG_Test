#include <iostream>

#include "core/UI.h"
#include "game/Game.h"

int main() {
    try {
        Game game;
        game.run();
    } catch (const ui::InputClosed&) {
        std::cout << "\nFarewell, adventurer.\n";
    }
    return 0;
}
