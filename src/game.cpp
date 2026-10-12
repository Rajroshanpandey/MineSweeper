#include "minecleaner/game.hpp"

#include <istream>
#include <ostream>
#include <string>

#include "minecleaner/ansi.hpp"
#include "minecleaner/board.hpp"
#include "minecleaner/input.hpp"
#include "minecleaner/ui.hpp"

namespace minecleaner {

    void Game::run() {
        while (const auto difficulty = chooseDifficulty()) {
            playRound(*difficulty);
        }
        out_ << ansi::clear << "Thanks for playing Minecleaner!\n";
    }

    std::optional<Difficulty> Game::chooseDifficulty() {
        while (true) {
            ui::renderMenu(out_);

            std::string line;
            if (!std::getline(in_, line)) return std::nullopt;  // EOF

            if (line == "1") return Difficulty::Easy;
            if (line == "2") return Difficulty::Intermediate;
            if (line == "3") return Difficulty::Hard;
            if (line == "q" || line == "Q") return std::nullopt;
        }
    }

    void Game::playRound(Difficulty difficulty) {
        Board board{configFor(difficulty)};
        std::string message;

        while (board.state() == GameState::Playing) {
            out_ << ansi::clear;
            ui::renderBoard(out_, board);
            if (!message.empty()) out_ << ansi::yellow << "  " << message << ansi::reset << '\n';
            message.clear();
            out_ << "  Command (h = help) > " << std::flush;

            std::string line;
            if (!std::getline(in_, line)) return;  // EOF -> abandon the round

            const auto cmd = parseCommand(line, board.config());
            if (!cmd) {
                message = "Invalid input. Try: r 3 4   or   f 3 4   (h for help)";
                continue;
            }

            switch (cmd->action) {
                case Action::Reveal: board.reveal(cmd->row, cmd->col); break;
                case Action::Flag:   board.toggleFlag(cmd->row, cmd->col); break;
                case Action::Chord:  board.chord(cmd->row, cmd->col); break;
                case Action::Help: {
                                       ui::renderHelp(out_);
                                       std::string wait;
                                       std::getline(in_, wait);
                                       break;
                                   }
                case Action::Quit:   return;
            }
        }

        // Round finished: show the final board.
        out_ << ansi::clear;
        ui::renderBoard(out_, board);
        out_ << "  Time: " << board.elapsed().count() << "s\n\n  Press Enter to return to the menu..."
            << std::flush;
        std::string wait;
        std::getline(in_, wait);
    }

}  // namespace minecleaner

