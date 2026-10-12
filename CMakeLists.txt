cmake_minimum_required(VERSION 3.16)
project(Minecleaner VERSION 1.0.0 LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

    option(MINECLEANER_BUILD_TESTS "Build the unit tests" ON)

# ---- warnings ---------------------------------------------------------------
    add_library(minecleaner_warnings INTERFACE)
    if(MSVC)
    target_compile_options(minecleaner_warnings INTERFACE /W4)
    else()
    target_compile_options(minecleaner_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# ---- core library (everything except main) ----------------------------------
    add_library(minecleaner_core
            src/board.cpp
            src/input.cpp
            src/ui.cpp
            src/game.cpp
            )
    target_include_directories(minecleaner_core PUBLIC include)
target_link_libraries(minecleaner_core PUBLIC minecleaner_warnings)

# ---- game executable --------------------------------------------------------
    add_executable(minecleaner src/main.cpp)
target_link_libraries(minecleaner PRIVATE minecleaner_core)

# ---- tests ------------------------------------------------------------------
    if(MINECLEANER_BUILD_TESTS)
    enable_testing()
    add_executable(minecleaner_tests tests/test_main.cpp)
    target_link_libraries(minecleaner_tests PRIVATE minecleaner_core)
    add_test(NAME minecleaner_tests COMMAND minecleaner_tests)
endif()

