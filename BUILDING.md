
# Building a new ROM

This document explains how to build the tools to disect the debug
ROM image, how to diesect the ROM, how to reassemble it, and how
to verify the result.

## Tool Overview

DyneSect comes with three tools

 - `dynesect` reads the debug image, analyses it, and writes
   assembler C, and C++ files based on its findings
 - `dynepatch` supports the Norcroft toolchain in recreating the ROM
 - `dynediff` compares the generated ROM to the original ROM, allowing
   for binary differences that are identical in function

##  Building the tools

All tools are build using CMake. Use the CMake options to set
the paths to the various resources at build time.
See `cmake/options.cmake`.

## Build a new ROM

TBD

## Verify the new ROM

TBD
