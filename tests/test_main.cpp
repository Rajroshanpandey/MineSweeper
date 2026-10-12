// Tiny dependency-free test runner: no GoogleTest/Catch2 needed.
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "minecleaner/board.hpp"
#include "minecleaner/game.hpp"
#include "minecleaner/input.hpp"

using namespace minecleaner;

namespace {

    int g_checks = 0;
    int g_failed = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            ++g_failed;                                                          \
            std::cerr << "    FAILED " << __FILE__ << ':' << __LINE__ << "  " #cond \
            << '\n';                                                   \
        }                                                                        \
    } while (false)

    struct TestCase {
        std::string           name;
        std::function<void()> fn;
    };

    // ---- helpers ---------------------------------------------------------------
    int countMines(const Board& b) {
        int n = 0;
        for (int r = 0; r < b.config().rows; ++r)
            for (int c = 0; c < b.config().cols; ++c)
                if (b.cell(r, c).mine) ++n;
        return n;
    }

    // ---- Board tests -----------------------------------------------------------
    void firstClickIsAlwaysSafe() {
        for (unsigned seed = 1; seed <= 300; ++seed) {
            Board b{configFor(Difficulty::Hard), seed};
            b.reveal(8, 15);
            CHECK(b.state() != GameState::Lost);
            CHECK(b.cell(8, 15).revealed);
            // The whole 3x3 neighbourhood is mine-free, so a zero opens up.
            CHECK(b.cell(8, 15).adjacent == 0);
        }
    }

    void correctNumberOfMinesPerDifficulty() {
        for (const auto d : {Difficulty::Easy, Difficulty::Intermediate, Difficulty::Hard}) {
            Board b{configFor(d), 42u};
            b.reveal(0, 0);
            CHECK(countMines(b) == configFor(d).mines);
        }
    }

    void adjacentCountsMatchBruteForce() {
        Board b{configFor(Difficulty::Intermediate), 7u};
        b.reveal(5, 5);
        const auto& cfg = b.config();
        for (int r = 0; r < cfg.rows; ++r) {
            for (int c = 0; c < cfg.cols; ++c) {
                int expected = 0;
                for (int dr = -1; dr <= 1; ++dr)
                    for (int dc = -1; dc <= 1; ++dc)
                        if ((dr || dc) && b.inBounds(r + dr, c + dc) && b.cell(r + dr, c + dc).mine)
                            ++expected;
                CHECK(b.cell(r, c).adjacent == expected);
            }
        }
    }

    void flaggingWorks() {
        Board b{configFor(Difficulty::Easy), 1u};
        CHECK(b.flagsLeft() == 10);
        b.toggleFlag(0, 0);
        CHECK(b.cell(0, 0).flagged);
        CHECK(b.flagsLeft() == 9);
        b.toggleFlag(0, 0);
        CHECK(!b.cell(0, 0).flagged);
        CHECK(b.flagsLeft() == 10);
    }

    void flaggedCellCannotBeRevealed() {
        Board b{configFor(Difficulty::Easy), 1u};
        b.toggleFlag(2, 2);
        b.reveal(2, 2);
        CHECK(!b.cell(2, 2).revealed);
        CHECK(b.revealedCount() == 0);
    }

    void cannotFlagRevealedCell() {
        Board b{configFor(Difficulty::Easy), 3u};
        b.reveal(4, 4);
        b.toggleFlag(4, 4);
        CHECK(!b.cell(4, 4).flagged);
        CHECK(b.flagsLeft() == 10);
    }

    void outOfBoundsIsIgnored() {
        Board b{configFor(Difficulty::Easy), 3u};
        b.reveal(-1, 0);
        b.reveal(0, 99);
        b.toggleFlag(50, 50);
        b.chord(-5, -5);
        CHECK(b.state() == GameState::Playing);
        CHECK(b.revealedCount() == 0);
    }

    void steppingOnMineLoses() {
        Board b{configFor(Difficulty::Easy), 11u};
        b.reveal(4, 4);
        bool done = false;
        for (int r = 0; r < 9 && !done; ++r)
            for (int c = 0; c < 9 && !done; ++c)
                if (b.cell(r, c).mine) {
                    b.reveal(r, c);
                    done = true;
                }
        CHECK(done);
        CHECK(b.state() == GameState::Lost);
        CHECK(b.exploded().has_value());
    }

    void revealingAllSafeCellsWins() {
        Board b{configFor(Difficulty::Hard), 99u};
        b.reveal(8, 15);
        for (int r = 0; r < 16; ++r)
            for (int c = 0; c < 30; ++c)
                if (!b.cell(r, c).mine) b.reveal(r, c);
        CHECK(b.state() == GameState::Won);
        CHECK(b.flagsLeft() == 0);  // mines get auto-flagged on a win
    }

    void noActionsAfterGameOver() {
        Board b{configFor(Difficulty::Easy), 5u};
        b.reveal(4, 4);
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c)
                if (b.cell(r, c).mine) { b.reveal(r, c); r = 9; break; }
        const int before = b.revealedCount();
        b.reveal(0, 0);
        b.toggleFlag(0, 0);
        CHECK(b.revealedCount() == before);
        CHECK(!b.cell(0, 0).flagged);
    }

    void chordRevealsNeighboursWhenFlagsMatch() {
        Board b{configFor(Difficulty::Intermediate), 21u};
        b.reveal(8, 8);

        // Find a revealed number that still has hidden safe neighbours.
        for (int r = 0; r < 16; ++r) {
            for (int c = 0; c < 16; ++c) {
                const Cell& cell = b.cell(r, c);
                if (!cell.revealed || cell.adjacent == 0) continue;

                bool hasHiddenSafe = false;
                for (int dr = -1; dr <= 1; ++dr)
                    for (int dc = -1; dc <= 1; ++dc)
                        if ((dr || dc) && b.inBounds(r + dr, c + dc)) {
                            const Cell& n = b.cell(r + dr, c + dc);
                            if (!n.mine && !n.revealed) hasHiddenSafe = true;
                        }
                if (!hasHiddenSafe) continue;

                // Chord with too few flags does nothing.
                const int before = b.revealedCount();
                b.chord(r, c);
                CHECK(b.revealedCount() == before);

                // Flag exactly the mines, then chord.
                for (int dr = -1; dr <= 1; ++dr)
                    for (int dc = -1; dc <= 1; ++dc)
                        if ((dr || dc) && b.inBounds(r + dr, c + dc) && b.cell(r + dr, c + dc).mine)
                            b.toggleFlag(r + dr, c + dc);
                b.chord(r, c);

                CHECK(b.state() != GameState::Lost);
                for (int dr = -1; dr <= 1; ++dr)
                    for (int dc = -1; dc <= 1; ++dc)
                        if ((dr || dc) && b.inBounds(r + dr, c + dc) && !b.cell(r + dr, c + dc).mine)
                            CHECK(b.cell(r + dr, c + dc).revealed);
                return;
            }
        }
        CHECK(false);  // no suitable cell found - would mean the test setup is wrong
    }

    // ---- input parsing tests ---------------------------------------------------
    void parserAcceptsValidCommands() {
        const Config cfg = configFor(Difficulty::Easy);

        auto reveal = parseCommand("r 3 4", cfg);
        CHECK(reveal && reveal->action == Action::Reveal && reveal->row == 2 && reveal->col == 3);

        auto bare = parseCommand("9 9", cfg);
        CHECK(bare && bare->action == Action::Reveal && bare->row == 8 && bare->col == 8);

        auto flag = parseCommand("F 1 1", cfg);  // case-insensitive
        CHECK(flag && flag->action == Action::Flag);

        auto chord = parseCommand("  c   2   2  ", cfg);
        CHECK(chord && chord->action == Action::Chord);

        CHECK(parseCommand("q", cfg)->action == Action::Quit);
        CHECK(parseCommand("help", cfg)->action == Action::Help);
    }

    void parserRejectsInvalidCommands() {
        const Config cfg = configFor(Difficulty::Easy);
        CHECK(!parseCommand("", cfg));
        CHECK(!parseCommand("hello", cfg));
        CHECK(!parseCommand("r 3", cfg));
        CHECK(!parseCommand("r a b", cfg));
        CHECK(!parseCommand("r 0 1", cfg));    // 1-based, so 0 is invalid
        CHECK(!parseCommand("r 10 1", cfg));   // easy has 9 rows
        CHECK(!parseCommand("1 10", cfg));     // easy has 9 cols
    }

    // ---- end-to-end game test --------------------------------------------------
    void gameLoopRunsFromScriptedInput() {
        std::istringstream in{"1\nf 1 1\nr 5 5\nbogus\nq\nq\n"};
        std::ostringstream out;
        Game game{in, out};
        game.run();

        const std::string text = out.str();
        CHECK(text.find("Select difficulty") != std::string::npos);
        CHECK(text.find("[EASY]") != std::string::npos);
        CHECK(text.find("Invalid input") != std::string::npos);
        CHECK(text.find("Thanks for playing") != std::string::npos);
    }

    void gameLoopSurvivesEndOfInput() {
        std::istringstream in{"3\nr 8 15\n"};  // stream ends mid-game
        std::ostringstream out;
        Game game{in, out};
        game.run();
        CHECK(out.str().find("[HARD]") != std::string::npos);
    }

}  // namespace

int main() {
    const std::vector<TestCase> tests = {
        {"first click is always safe",              firstClickIsAlwaysSafe},
        {"correct mine count per difficulty",       correctNumberOfMinesPerDifficulty},
        {"adjacent counts match brute force",       adjacentCountsMatchBruteForce},
        {"flagging toggles and counts",             flaggingWorks},
        {"flagged cell cannot be revealed",         flaggedCellCannotBeRevealed},
        {"cannot flag a revealed cell",             cannotFlagRevealedCell},
        {"out-of-bounds input is ignored",          outOfBoundsIsIgnored},
        {"stepping on a mine loses",                steppingOnMineLoses},
        {"revealing all safe cells wins",           revealingAllSafeCellsWins},
        {"no actions after game over",              noActionsAfterGameOver},
        {"chord reveals neighbours",                chordRevealsNeighboursWhenFlagsMatch},
        {"parser accepts valid commands",           parserAcceptsValidCommands},
        {"parser rejects invalid commands",         parserRejectsInvalidCommands},
        {"game loop with scripted input",           gameLoopRunsFromScriptedInput},
        {"game loop survives end of input",         gameLoopSurvivesEndOfInput},
    };

    for (const auto& [name, fn] : tests) {
        const int failedBefore = g_failed;
        fn();
        std::cout << (g_failed == failedBefore ? "  [PASS] " : "  [FAIL] ") << name << '\n';
    }

    std::cout << "\n" << (g_checks - g_failed) << "/" << g_checks << " checks passed\n";
    return g_failed == 0 ? 0 : 1;
}

