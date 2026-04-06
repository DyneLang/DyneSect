cmake_minimum_required(VERSION 3.20)
project(DyneSect LANGUAGES CXX)

# --- Language standard --------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- User-configurable paths --------------------------------------------
include(cmake/options.cmake)

# --- Dependencies -------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
# Keep gtest's install targets out of our install
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Tools --------------------------------------------------------------
add_subdirectory(src/dynesect)
add_subdirectory(src/dynediff)
add_subdirectory(src/dynepatch)

# --- Tests --------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
