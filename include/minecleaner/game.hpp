#pragma once

#include <iosfwd>
#include <optional>

#include "minecleaner/types.hpp"

namespace minecleaner {

    // Drives the menu -> round -> menu loop. Streams are injected so the whole
    // game can be exercised from unit tests with std::stringstream.
    class Game {
        public:
            Game(std::istream& in, std::ostream& out) : in_{in}, out_{out} {}

            void run();

        private:
            std::istream& in_;
            std::ostream& out_;

            [[nodiscard]] std::optional<Difficulty> chooseDifficulty();
            void playRound(Difficulty difficulty);
    };

}  // namespace minecleaner

