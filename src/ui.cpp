#include "minecleaner/ui.hpp"

#include <cstddef>
#include <iomanip>
#include <ostream>
#include <string>

#include "minecleaner/ansi.hpp"

namespace minecleaner::ui {

    namespace {

        void renderCell(std::ostream& os, const Board& board, int r, int c) {
            const Cell& cell = board.cell(r, c);
            const bool lost = board.state() == GameState::Lost;

            if (lost) {
                const auto& boom = board.exploded();
                if (boom && boom->first == r && boom->second == c) {
                    os << ansi::redBg << '*' << ansi::reset;
                    return;
                }
                if (cell.mine && !cell.flagged) { os << ansi::red << '*' << ansi::reset; return; }
                if (!cell.mine && cell.flagged) { os << ansi::red << 'X' << ansi::reset; return; }
            }

            if (cell.flagged) {
                os << ansi::yellow << ansi::bold << 'F' << ansi::reset;
            } else if (!cell.revealed) {
                os << ansi::dim << '#' << ansi::reset;
            } else if (cell.adjacent == 0) {
                os << ' ';
            } else {
                os << ansi::numberColour[static_cast<std::size_t>(cell.adjacent)] << ansi::bold
                    << cell.adjacent << ansi::reset;
            }
        }

    }  // namespace

    void renderMenu(std::ostream& os) {
        os << ansi::clear << ansi::bold << ansi::cyan
            << R"(
   __  __ _                  _
  |  \/  (_)_ _  ___ __ _ ___| |___ __ _ _ _  ___ _ _
  | |\/| | | ' \/ -_) _| / -_) / -_) _` | ' \/ -_) '_|
  |_|  |_|_|_||_\___\__|_\___|_\___\__,_|_||_\___|_|
)" << ansi::reset << "\n  Select difficulty:\n\n";

            for (const auto d : {Difficulty::Easy, Difficulty::Intermediate, Difficulty::Hard}) {
                const Config cfg = configFor(d);
                const int number = static_cast<int>(d) + 1;
                os << "   " << number << ") " << std::left << std::setw(14) << cfg.name << std::right
                    << std::setw(2) << cfg.rows << " x " << std::setw(2) << cfg.cols << "   "
                    << cfg.mines << " mines\n";
            }
        os << "   q) Quit\n\n  > " << std::flush;
    }

    void renderBoard(std::ostream& os, const Board& board) {
        const Config& cfg = board.config();
        const std::string border = "    +" + std::string(static_cast<std::size_t>(cfg.cols) * 3, '-') + "+\n";

        os << ansi::bold << "  MINECLEANER" << ansi::reset << "  [" << cfg.name << "]\n"
            << "  Mines left: " << ansi::yellow << std::setw(3) << board.flagsLeft() << ansi::reset
            << "    Time: " << ansi::cyan << std::setw(3) << board.elapsed().count() << 's'
            << ansi::reset << "\n\n     ";

        for (int c = 0; c < cfg.cols; ++c) {
            os << ansi::dim << std::setw(3) << (c + 1) << ansi::reset;
        }
        os << '\n' << border;

        for (int r = 0; r < cfg.rows; ++r) {
            os << ansi::dim << std::setw(3) << (r + 1) << ansi::reset << " |";
            for (int c = 0; c < cfg.cols; ++c) {
                os << ' ';
                renderCell(os, board, r, c);
                os << ' ';
            }
            os << "|\n";
        }
        os << border << '\n';

        switch (board.state()) {
            case GameState::Won:
                os << ansi::green << ansi::bold << "  *** You cleared the field! You win! ***"
                    << ansi::reset << '\n';
                break;
            case GameState::Lost:
                os << ansi::red << ansi::bold << "  *** BOOM! You hit a mine. Game over. ***"
                    << ansi::reset << '\n';
                break;
            case GameState::Playing:
                break;
        }
    }

    void renderHelp(std::ostream& os) {
        os << ansi::bold << "\n  HOW TO PLAY\n" << ansi::reset
            << "  -----------\n"
            << "  r <row> <col>   reveal a cell (or just type: <row> <col>)\n"
            << "  f <row> <col>   flag / unflag a suspected mine\n"
            << "  c <row> <col>   chord: if a number has enough flags around it,\n"
            << "                  reveal all of its other neighbours\n"
            << "  h               show this help\n"
            << "  q               quit to the menu\n\n"
            << "  Numbers show how many mines touch that cell (8 directions).\n"
            << "  Reveal every safe cell to win. Your first click is always safe.\n\n"
            << "  Press Enter to continue..." << std::flush;
    }

}  // namespace minecleaner::ui

