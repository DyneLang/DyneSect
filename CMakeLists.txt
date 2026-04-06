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

# --- Path definitions ---------------------------------------------------
# Propagates all DSECT_* path options to a target as preprocessor string macros.
function(dsect_apply_options target)
    target_compile_definitions(${target} PRIVATE
        DSECT_TOOLS_PATH="${DSECT_TOOLS_PATH}"
        DSECT_DEBUG_IMAGE_PATH="${DSECT_DEBUG_IMAGE_PATH}"
        DSECT_DDK_INCLUDE_PATH="${DSECT_DDK_INCLUDE_PATH}"
        DSECT_WORK_PATH="${DSECT_WORK_PATH}"
    )
endfunction()

# --- Tools --------------------------------------------------------------
add_subdirectory(src/dynesect)
add_subdirectory(src/dynediff)
add_subdirectory(src/dynepatch)

# --- ROM build pipeline (Norcroft toolchain) ----------------------------
include(cmake/rom_build.cmake)

# --- Tests --------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
