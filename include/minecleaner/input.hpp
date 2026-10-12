#pragma once

#include <optional>
#include <string>

#include "minecleaner/types.hpp"

namespace minecleaner {

    // Turns a line typed by the player into a Command.
    // Returns std::nullopt for malformed input or out-of-range coordinates.
    // Player coordinates are 1-based; the returned Command is 0-based.
    [[nodiscard]] std::optional<Command> parseCommand(const std::string& line, const Config& cfg);

}  // namespace minecleaner

