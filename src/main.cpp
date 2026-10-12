#include <iostream>

#include "minecleaner/game.hpp"

int main() {
    minecleaner::Game game{std::cin, std::cout};
    game.run();
    return 0;
}

