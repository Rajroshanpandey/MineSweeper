# Minecleaner

A terminal Minesweeper written in modern C++17, with three difficulty levels,
colour output, a timer, flood-fill reveal, chording, and unit tests.

## Difficulty levels

| Level        | Grid    | Mines |
|--------------|---------|-------|
| Easy         | 9 x 9   | 10    |
| Intermediate | 16 x 16 | 40    |
| Hard         | 16 x 30 | 99    |

> Hard mode is about 96 columns wide, so use a reasonably wide terminal window.

## Project layout

```
minecleaner/
├── CMakeLists.txt
├── Makefile                      # fallback if you don't have CMake
├── README.md
├── include/minecleaner/
│   ├── types.hpp                 # enums, Config, Cell, Command
│   ├── ansi.hpp                  # terminal colour codes
│   ├── board.hpp                 # game rules (no I/O)
│   ├── input.hpp                 # command parsing
│   ├── ui.hpp                    # rendering
│   └── game.hpp                  # menu + round loop
├── src/
│   ├── main.cpp
│   ├── board.cpp
│   ├── input.cpp
│   ├── ui.cpp
│   └── game.cpp
└── tests/
    └── test_main.cpp             # dependency-free unit tests
```

## Build and run

### With CMake (recommended)

```bash
cmake -S . -B build
cmake --build build
./build/minecleaner
```

Run the tests:

```bash
cd build && ctest --output-on-failure
```

### With plain make

```bash
make run     # build and play
make test    # build and run unit tests
make clean
```

Requirements: a C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+) and a terminal
that understands ANSI colours (Linux, macOS, Windows Terminal, WSL).

## How to play

| Command         | Action                                                |
|-----------------|-------------------------------------------------------|
| `r <row> <col>` | Reveal a cell (typing just `<row> <col>` also works)  |
| `f <row> <col>` | Flag / unflag a suspected mine                        |
| `c <row> <col>` | Chord: reveal the neighbours of a satisfied number    |
| `h`             | Help                                                  |
| `q`             | Quit to the menu                                      |

Rows and columns start at 1. Your first click is always safe.

## Design notes

- `Board` contains all rules and knows nothing about the terminal, so it is easy to test.
  It takes an optional seed for reproducible layouts.
- Mines are placed after the first click, away from that cell and its neighbours.
- Revealing uses an iterative queue-based flood fill (no recursion, so no stack
  overflow risk on big boards).
- `Game` takes `std::istream&` / `std::ostream&`, so the whole game can be driven
  from tests with `std::stringstream`.

## Modern C++ features used

`enum class`, `constexpr` functions, `std::optional`, structured bindings,
`std::string_view`, `inline constexpr` variables, `[[nodiscard]]`, generic
lambdas / templated callbacks, `std::chrono`, `std::mt19937`, raw string
literals, and namespaces.

