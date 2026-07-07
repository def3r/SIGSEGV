cmake_minimum_required(VERSION 3.20)

# nvq++ must be the CXX compiler — set before project().
# Pass on the command line:  cmake -DCMAKE_CXX_COMPILER=$(which nvq++) -B build
if(NOT CMAKE_CXX_COMPILER)
    find_program(NVQPP nvq++ REQUIRED)
    set(CMAKE_CXX_COMPILER "${NVQPP}" CACHE FILEPATH "C++ compiler" FORCE)
endif()

project(c2cudaq LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nvq++ does not generate Make dependency files (.d).
# Override the compile rule to omit -MD/-MF flags entirely.
set(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")

# ── Library ──────────────────────────────────────────────────────────────────
add_library(c2cudaq STATIC
    src/qubo.cpp
    src/arith.cpp
    src/qaoa.cpp
    src/grover.cpp
    src/shor.cpp
    src/bridge.cpp
)

target_include_directories(c2cudaq PUBLIC include)

# ── Tests ────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)

# ── Examples ─────────────────────────────────────────────────────────────────
add_subdirectory(examples)
