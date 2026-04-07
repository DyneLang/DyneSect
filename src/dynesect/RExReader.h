#pragma once

#include "Label.h"
#include "Memory.h"

#include <string>

// ----------------------------------------------------------------------------
// REx (ROM Extension) reader
//
// Reads a Newton REx image file and loads it into `memory` at the address
// given by the REx header. The REx is a self-describing binary blob that
// extends the base ROM; it immediately follows the AIF image in the Newton
// address space.
//
// Call read_aif() first so that the base ROM block already exists in `memory`
// before calling read_rex().
//
// Throws std::runtime_error on any file or format error.
// ----------------------------------------------------------------------------
void read_rex(const std::string& path, int base, Memory& memory, LabelMap& labels);
