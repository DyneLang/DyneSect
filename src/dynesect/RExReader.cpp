#include "RExReader.h"

#include "main.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

// ----------------------------------------------------------------------------
// REx data is copied verbatim right after the core OS read form the AIF file
// ----------------------------------------------------------------------------

// ---- helpers ----------------------------------------------------------------

static std::string file_error(const std::string& context, const std::string& path)
{
    return std::format("{}: {} -- {}", context, path, strerror(errno));
}

// ---- public -----------------------------------------------------------------

void read_rex(const std::string& path, int base, Memory& memory, LabelMap& labels)
{
    // --- open ----------------------------------------------------------------
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        throw std::runtime_error(file_error("Cannot open REx file", path));

    // --- get file size -------------------------------------------------------
    fseek(f, 0, SEEK_END);
    int total_size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);

    // --- load image into Memory ----------------------------------------------
    std::vector<uint8_t> image(total_size);
    if (fread(image.data(), 1, total_size, f) != total_size) {
        fclose(f);
        throw std::runtime_error(file_error("Failed to read REx image data", path));
    }

    memory.block_at(0x00000000)->write_block(base, image);

    // --- symbol / package table ----------------------------------------------
    // The REx contains a list of public packages and kernel patches described
    // by a table of REx entries starting at offset 0x14. Each entry carries a
    // tag, offset, and size. Named entries can be added to `labels` once the
    // entry layout is confirmed against the Newton binary.
    //
    // TODO: iterate REx entries and populate `labels` with FromREx flag.

    (void)labels;   // suppress unused-parameter warning until TODO is filled in

    fclose(f);
}
