
# Paths to the Norcroft toolchain (compiler, assembler, linker).
# These are required only for the ROM build/verify targets, not for the
# DyneSect tools themselves.
set(DSECT_TOOLS_PATH "" CACHE PATH
    "Directory containing the Norcroft ARM C++ compiler toolchain")

# Path to the Newton AIF and REx file from the Newton Developer Kit.
set(DSECT_DEBUG_IMAGE_PATH "" CACHE PATH
    "Path to the Newton ROM AIF image file")

# Path to the DDK C++ header files.
set(DSECT_DDK_INCLUDE_PATH "" CACHE PATH
    "Directory containing the Newton DDK C++ header files")

# Working directory where dynesect writes its generated source files.
set(DSECT_WORK_PATH "" CACHE PATH
    "Output directory for generated source files")
