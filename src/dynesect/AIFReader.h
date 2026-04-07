#pragma once

#include "Label.h"
#include "Memory.h"

#include <string>

// ----------------------------------------------------------------------------
// AIF (ARM Image Format) reader
//
// Reads an AIF file produced by the Norcroft toolchain and:
//   - loads the read-only (code+rodata) image into a MemoryBlock in `memory`
//     at the load address recorded in the AIF header (typically 0x00000000)
//   - populates `labels` with every entry from the AIF symbol table
//
// The REx image is handled separately; call read_rex() for that.
//
// Reference: ARM AIF specification, ARM DUI 0041 §2.
// ----------------------------------------------------------------------------

// Load the AIF image into `memory` and extract its symbol table into `labels`.
// Throws std::runtime_error on any file or format error.
uint32_t read_aif(const std::string& path, Memory& memory, LabelMap& labels);
