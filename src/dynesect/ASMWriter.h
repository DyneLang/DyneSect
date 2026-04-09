#pragma once

#include "Label.h"
#include "Memory.h"

#include <string>

// ----------------------------------------------------------------------------
// ASMWriter
//
// Writes a range of ROM memory as Norcroft ARM assembler source.
// Labels from the LabelMap are inserted before the DCD line for their address.
// ----------------------------------------------------------------------------

// Write the address range [start, end) from `memory` as a Norcroft assembler
// file to `path`. Labels from `labels` are emitted before the DCD line for
// each labelled address. If the file already exists and the content is
// identical, the file is not touched (preserving its modification timestamp so
// ARM6asm skips it on the next build).
void write_asm(const std::string& path,
               uint32_t start, uint32_t end,
               const Memory& memory,
               const LabelMap& labels);
