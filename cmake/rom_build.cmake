
# ROM assembly and link pipeline using the Norcroft toolchain.
#
# Chunk layout is driven by rom_chunks.json at the project root.
# Editing that file and running cmake --build is sufficient -- CMake
# re-configures automatically because the file is listed as a configure
# dependency.
#
# Requires:
#   DSECT_TOOLS_PATH  -- directory containing ARM6asm and ARMLink
#   DSECT_WORK_PATH   -- directory where dynesect writes .s files and where
#                        .o files and the final ROM will be placed
#
# Targets defined here:
#   rom_assemble  -- assembles all changed .s files into .o files (parallel-safe)
#   rom_link      -- links the .o files into new_rom.bin
#   rom           -- top-level target: assemble + link

if(NOT DSECT_TOOLS_PATH)
    message(STATUS "ROM build targets not configured: set DSECT_TOOLS_PATH to the Norcroft toolchain directory")
    return()
endif()

if(NOT DSECT_WORK_PATH)
    message(STATUS "ROM build targets not configured: set DSECT_WORK_PATH to the dynesect output directory")
    return()
endif()

set(ARM6ASM "${DSECT_TOOLS_PATH}/ARM6asm")
set(ARMLINK "${DSECT_TOOLS_PATH}/ARMLink")

# Re-configure automatically whenever the chunk list changes.
set(ROM_CHUNKS_JSON "${CMAKE_SOURCE_DIR}/rom_chunks.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ROM_CHUNKS_JSON}")

# Parse the chunk list.
file(READ "${ROM_CHUNKS_JSON}" _json)
string(JSON _chunk_count LENGTH "${_json}" "chunks")
math(EXPR _last_index "${_chunk_count} - 1")

set(ROM_OBJECTS "")

foreach(i RANGE ${_last_index})
    string(JSON _name GET "${_json}" "chunks" ${i} "name")

    if(${_name} MATCHES ".*/")
        # set a new subdirectory for all following files
        # message(FATAL_ERROR, "Dir found!")
    elseif(${_source} MATCHES ".*\.s")
        # ARM32 assembler file
    elseif(${_source} MATCHES ".*\.cpp")
        # C++ file (patch __arm command)
    elseif(${_source} MATCHES ".*\.c")
        # C file (patch __arm command)
    endif()

    set(_src "${DSECT_WORK_PATH}/${_name}")
    set(_obj "${DSECT_WORK_PATH}/${_name}.o")

    # Each block is an independent command so Ninja/Make can parallelise them.
    add_custom_command(
        OUTPUT  "${_obj}"
        COMMAND "${ARM6ASM}" "${_src}" -o "${_obj}"
        DEPENDS "${_src}"
        COMMENT "ARM6asm ${_name}"
        VERBATIM
    )

    list(APPEND ROM_OBJECTS "${_obj}")
endforeach()

add_custom_target(rom_assemble DEPENDS ${ROM_OBJECTS})

# Link all object files into the final ROM binary.
set(ROM_BIN "${DSECT_WORK_PATH}/new_rom.bin")

add_custom_command(
    OUTPUT  "${ROM_BIN}"
    COMMAND "${ARMLINK}" ${ROM_OBJECTS} -map -bin -o "${ROM_BIN}"
    DEPENDS ${ROM_OBJECTS}
    COMMENT "ARMLink -> new_rom.bin"
    VERBATIM
)

add_custom_target(rom_link DEPENDS "${ROM_BIN}")

# Convenience top-level target.
add_custom_target(rom DEPENDS rom_link)
