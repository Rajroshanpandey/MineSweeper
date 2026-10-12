#include "minecleaner/input.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace minecleaner {

    std::optional<Command> parseCommand(const std::string& line, const Config& cfg) {
        std::istringstream in{line};
        std::string first;
        if (!(in >> first)) return std::nullopt;

        std::transform(first.begin(), first.end(), first.begin(),
                [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        if (first == "q" || first == "quit") return Command{Action::Quit};
        if (first == "h" || first == "help") return Command{Action::Help};

        Action action = Action::Reveal;
        int row = 0;
        int col = 0;

        if (first == "r" || first == "f" || first == "c") {
            action = (first == "r") ? Action::Reveal
                : (first == "f") ? Action::Flag
                : Action::Chord;
            if (!(in >> row >> col)) return std::nullopt;
        } else {
            // Plain "row col" means reveal.
            try {
                row = std::stoi(first);
            } catch (const std::exception&) {
                return std::nullopt;
            }
            if (!(in >> col)) return std::nullopt;
        }

        if (row < 1 || row > cfg.rows || col < 1 || col > cfg.cols) return std::nullopt;
        return Command{action, row - 1, col - 1};
    }

}  // namespace minecleaner

