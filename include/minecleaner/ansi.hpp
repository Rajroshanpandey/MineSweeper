#pragma once

#include <array>
#include <string_view>

namespace minecleaner::ansi {

inline constexpr std::string_view reset  = "\033[0m";
inline constexpr std::string_view bold   = "\033[1m";
inline constexpr std::string_view dim    = "\033[2m";
inline constexpr std::string_view red    = "\033[31m";
inline constexpr std::string_view green  = "\033[32m";
inline constexpr std::string_view yellow = "\033[33m";
inline constexpr std::string_view cyan   = "\033[36m";
inline constexpr std::string_view redBg  = "\033[41;97m";
inline constexpr std::string_view clear  = "\033[2J\033[H";

// Classic Minesweeper number colours; index = number of adjacent mines.
inline constexpr std::array<std::string_view, 9> numberColour = {
    "",          // 0 (never printed)
    "\033[94m",  // 1 bright blue
    "\033[32m",  // 2 green
    "\033[91m",  // 3 bright red
    "\033[34m",  // 4 dark blue
    "\033[31m",  // 5 dark red
    "\033[36m",  // 6 cyan
    "\033[35m",  // 7 magenta
    "\033[90m",  // 8 grey
};

}  // namespace minecleaner::ansi

