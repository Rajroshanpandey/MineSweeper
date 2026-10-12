#pragma once

#include <iosfwd>

#include "minecleaner/board.hpp"

namespace minecleaner::ui {

    void renderMenu(std::ostream& os);
    void renderBoard(std::ostream& os, const Board& board);
    void renderHelp(std::ostream& os);

}  // namespace minecleaner::ui

