#include "minecleaner/board.hpp"

#include <algorithm>
#include <cstdlib>
#include <queue>
#include <stdexcept>

namespace minecleaner {

    Board::Board(const Config& cfg, std::optional<unsigned> seed)
        : cfg_{cfg},
        grid_(static_cast<std::size_t>(cfg.rows) * static_cast<std::size_t>(cfg.cols)),
        rng_{seed.value_or(std::random_device{}())} {}

    // ---------------------------------------------------------------------------
    // Queries
    // ---------------------------------------------------------------------------
    bool Board::inBounds(int r, int c) const noexcept {
        return r >= 0 && r < cfg_.rows && c >= 0 && c < cfg_.cols;
    }

    const Cell& Board::cell(int r, int c) const {
        if (!inBounds(r, c)) throw std::out_of_range{"Board::cell: out of range"};
        return grid_[indexOf(r, c)];
    }

    std::chrono::seconds Board::elapsed() const {
        if (!started_) return std::chrono::seconds{0};
        const auto end = (state_ == GameState::Playing) ? Clock::now() : endTime_;
        return std::chrono::duration_cast<std::chrono::seconds>(end - startTime_);
    }

    int Board::safeCellCount() const noexcept { return cfg_.rows * cfg_.cols - cfg_.mines; }

    std::size_t Board::indexOf(int r, int c) const noexcept {
        return static_cast<std::size_t>(r) * static_cast<std::size_t>(cfg_.cols) +
            static_cast<std::size_t>(c);
    }

    Cell& Board::at(int r, int c) { return grid_[indexOf(r, c)]; }

    // ---------------------------------------------------------------------------
    // Player actions
    // ---------------------------------------------------------------------------
    void Board::reveal(int r, int c) {
        if (state_ != GameState::Playing || !inBounds(r, c)) return;

        Cell& start = at(r, c);
        if (start.flagged || start.revealed) return;

        if (!started_) {  // the first click is always safe
            placeMines(r, c);
            started_   = true;
            startTime_ = Clock::now();
        }

        if (start.mine) {
            start.revealed = true;
            exploded_      = std::make_pair(r, c);
            finish(GameState::Lost);
            return;
        }

        // Flood fill: a zero opens all of its neighbours, which may be zeros too.
        std::queue<std::pair<int, int>> frontier;
        start.revealed = true;
        ++revealedCount_;
        frontier.emplace(r, c);

        while (!frontier.empty()) {
            const auto [cr, cc] = frontier.front();
            frontier.pop();
            if (at(cr, cc).adjacent != 0) continue;

            forEachNeighbour(cr, cc, [&](int nr, int nc) {
                    Cell& n = at(nr, nc);
                    if (n.revealed || n.flagged) return;
                    n.revealed = true;
                    ++revealedCount_;
                    frontier.emplace(nr, nc);
                    });
        }

        if (revealedCount_ == safeCellCount()) finish(GameState::Won);
    }

    void Board::toggleFlag(int r, int c) {
        if (state_ != GameState::Playing || !inBounds(r, c)) return;
        Cell& target = at(r, c);
        if (target.revealed) return;
        target.flagged = !target.flagged;
        flagCount_ += target.flagged ? 1 : -1;
    }

    void Board::chord(int r, int c) {
        if (state_ != GameState::Playing || !inBounds(r, c)) return;
        const Cell& target = at(r, c);
        if (!target.revealed || target.adjacent == 0) return;

        int flags = 0;
        forEachNeighbour(r, c, [&](int nr, int nc) { flags += at(nr, nc).flagged ? 1 : 0; });
        if (flags != target.adjacent) return;

        forEachNeighbour(r, c, [&](int nr, int nc) { reveal(nr, nc); });
    }

    // ---------------------------------------------------------------------------
    // Internals
    // ---------------------------------------------------------------------------
    void Board::finish(GameState result) {
        state_   = result;
        endTime_ = Clock::now();
        if (result == GameState::Won) {  // auto-flag the remaining mines
            for (Cell& c : grid_) {
                if (c.mine) c.flagged = true;
            }
            flagCount_ = cfg_.mines;
        }
    }

    // Mines are placed AFTER the first click so that cell and its neighbours
    // are guaranteed to be mine-free (the first click always opens an area).
    void Board::placeMines(int safeR, int safeC) {
        std::vector<std::size_t> candidates;
        candidates.reserve(grid_.size());

        for (int r = 0; r < cfg_.rows; ++r) {
            for (int c = 0; c < cfg_.cols; ++c) {
                const bool nearFirstClick = std::abs(r - safeR) <= 1 && std::abs(c - safeC) <= 1;
                if (!nearFirstClick) candidates.push_back(indexOf(r, c));
            }
        }

        std::shuffle(candidates.begin(), candidates.end(), rng_);
        for (int i = 0; i < cfg_.mines; ++i) {
            grid_[candidates[static_cast<std::size_t>(i)]].mine = true;
        }

        for (int r = 0; r < cfg_.rows; ++r) {
            for (int c = 0; c < cfg_.cols; ++c) {
                int count = 0;
                forEachNeighbour(r, c, [&](int nr, int nc) { count += at(nr, nc).mine ? 1 : 0; });
                at(r, c).adjacent = count;
            }
        }
    }

}  // namespace minecleaner

