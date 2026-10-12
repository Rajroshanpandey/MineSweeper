# Plain-make fallback for people who don't have CMake.
#   make          build the game  -> build/minecleaner
#   make test     build + run unit tests
#   make run      build + play
#   make clean

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
BUILD    := build

CORE_SRC := src/board.cpp src/input.cpp src/ui.cpp src/game.cpp
CORE_OBJ := $(CORE_SRC:src/%.cpp=$(BUILD)/%.o)

all: $(BUILD)/minecleaner

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

$(BUILD)/minecleaner: $(CORE_OBJ) $(BUILD)/main.o
	$(CXX) $^ -o $@

$(BUILD)/minecleaner_tests: $(CORE_OBJ) tests/test_main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(BUILD)/minecleaner_tests
	./$(BUILD)/minecleaner_tests

run: $(BUILD)/minecleaner
	./$(BUILD)/minecleaner

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)

.PHONY: all test run clean

