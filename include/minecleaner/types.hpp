#pragma once

#include <string_view>

namespace minecleaner {

    enum class Difficulty { Easy, Intermediate, Hard };
    enum class GameState  { Playing, Won, Lost };
    enum class Action     { Reveal, Flag, Chord, Help, Quit };

    struct Config {
        std::string_view name;
        int rows;
        int cols;
        int mines;
    };

    constexpr Config configFor(Difficulty d) noexcept {
        switch (d) {
            case Difficulty::Easy:         return {"EASY",         9,  9,  10};
            case Difficulty::Intermediate: return {"INTERMEDIATE", 16, 16, 40};
            case Difficulty::Hard:         return {"HARD",         16, 30, 99};
        }
        return {"EASY", 9, 9, 10};  // unreachable; keeps compilers happy
    }

    struct Cell {
        bool mine     = false;
        bool revealed = false;
        bool flagged  = false;
        int  adjacent = 0;  // mines in the 8 surrounding cells
    };

    // A parsed player command. row/col are 0-based.
    struct Command {
        Action action;
        int    row = 0;
        int    col = 0;
    };

}  // namespace minecleaner

