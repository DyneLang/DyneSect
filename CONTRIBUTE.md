# Contributing to DyneSect

This document describes the development tools and conventions used in DyneSect.

## Build System

We use **CMake** as our build system. CMake is used to:

- Build all DyneSect tools
- Build and verify the ROM
- Manage dependencies

## Language Standard

All code is written in **C++** up to the **C++20** standard. CMake files must
be configured to reflect this requirement (e.g., `set(CMAKE_CXX_STANDARD 20)`).

## Testing

We use the **Google Test** framework for unit testing our code. Tests should
be comprehensive and cover both expected behavior and edge cases.

## Dependencies

All external libraries must be pulled from their **original source** from
within the CMake files. This ensures reproducible builds and proper version
control. Use CMake's `FetchContent` or `ExternalProject` modules to manage
external dependencies.

## Code Style

We follow a **Clang-style** formatting with the following conventions:

- **Indentation**: 4 spaces (no tabs)
- Follow Clang's default style guidelines for other formatting decisions

A `.clang-format` file may be provided to enforce consistent formatting
across the codebase.
