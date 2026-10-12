#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "minecleaner/types.hpp"

namespace minecleaner {

    // Owns the grid and enforces every game rule. Knows nothing about I/O.
    class Board {
        public:
            // Pass a seed for reproducible layouts (used by the unit tests).
            explicit Board(const Config& cfg, std::optional<unsigned> seed = std::nullopt);

            // ---- queries ----------------------------------------------------------
            [[nodiscard]] GameState     state() const noexcept { return state_; }
            [[nodiscard]] const Config& config() const noexcept { return cfg_; }
            [[nodiscard]] int           flagsLeft() const noexcept { return cfg_.mines - flagCount_; }
            [[nodiscard]] int           revealedCount() const noexcept { return revealedCount_; }
            [[nodiscard]] bool          inBounds(int r, int c) const noexcept;
            [[nodiscard]] const Cell&   cell(int r, int c) const;
            [[nodiscard]] const std::optional<std::pair<int, int>>& exploded() const noexcept {
                return exploded_;
            }
            [[nodiscard]] std::chrono::seconds elapsed() const;

            // ---- player actions ---------------------------------------------------
            void reveal(int r, int c);      // first call also places the mines
            void toggleFlag(int r, int c);
            void chord(int r, int c);       // reveal neighbours of a satisfied number

        private:
            using Clock = std::chrono::steady_clock;

            Config                             cfg_;
            std::vector<Cell>                  grid_;
            std::mt19937                       rng_;
            GameState                          state_         = GameState::Playing;
            bool                               started_       = false;
            int                                revealedCount_ = 0;
            int                                flagCount_     = 0;
            std::optional<std::pair<int, int>> exploded_;
            Clock::time_point                  startTime_{};
            Clock::time_point                  endTime_{};

            [[nodiscard]] int          safeCellCount() const noexcept;
            [[nodiscard]] std::size_t  indexOf(int r, int c) const noexcept;
            [[nodiscard]] Cell&        at(int r, int c);

            template <typename Fn>
                void forEachNeighbour(int r, int c, Fn&& fn) const {
                    for (int dr = -1; dr <= 1; ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                            if (dr == 0 && dc == 0) continue;
                            const int nr = r + dr;
                            const int nc = c + dc;
                            if (inBounds(nr, nc)) fn(nr, nc);
                        }
                    }
                }

            void placeMines(int safeR, int safeC);
            void finish(GameState result);
    };

}  // namespace minecleaner

