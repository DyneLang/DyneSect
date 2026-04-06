
# ROM assembly and link pipeline using the Norcroft toolchain.
#
# Requires:
#   DSECT_TOOLS_PATH  -- directory containing ARM6asm and ARMLink
#   DSECT_WORK_PATH   -- directory where dynesect writes .s files and where
#                        .o files and the final ROM will be placed
#
# Targets defined here:
#   rom_assemble  -- assembles all changed rom_block_N.s files into .o files
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

# Assemble each block independently so only changed blocks are rebuilt.
set(ROM_BLOCKS 0 1 2 3 4 5 6 7)
set(ROM_OBJECTS "")

foreach(N IN LISTS ROM_BLOCKS)
    set(ASM_SRC "${DSECT_WORK_PATH}/rom_block_${N}.s")
    set(ASM_OBJ "${DSECT_WORK_PATH}/rom_block_${N}.s.o")

    add_custom_command(
        OUTPUT  "${ASM_OBJ}"
        COMMAND "${ARM6ASM}" "${ASM_SRC}" -o "${ASM_OBJ}"
        DEPENDS "${ASM_SRC}"
        COMMENT "ARM6asm rom_block_${N}.s"
        VERBATIM
    )

    list(APPEND ROM_OBJECTS "${ASM_OBJ}")
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
