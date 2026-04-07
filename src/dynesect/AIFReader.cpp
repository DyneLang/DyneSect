#include "AIFReader.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

// ----------------------------------------------------------------------------
// AIF header layout (ARM DUI 0041)
//
// 32 words (128 bytes) at the start of the file. All fields are big-endian
// 32-bit words, consistent with the Newton OS address space.
//
// Word  Offset  Field
//   0   0x00    BL <decompress>  or NOP (0xE1A00000)
//   1   0x04    BL <self-relocate> or NOP
//   2   0x08    BL <zero-init>   or NOP
//   3   0x0C    BL <entry>       or NOP  (branch to entry point)
//   4   0x10    SWI OS_Exit      (0xEF000011)
//   5   0x14    RO size          (read-only area, bytes)
//   6   0x18    RW size          (read-write area, bytes)
//   7   0x1C    Debug size       (debug area, bytes; 0 if none)
//   8   0x20    Zero-init size   (BSS, bytes)
//   9   0x24    Debug type       (0 = none, 1 = ASD, 2 = DWARF …)
//  10   0x28    Image base       (load address)
//  11   0x2C    Work space       (minimum stack, bytes)
//  12   0x30    Address mode     (26 or 32)
//  13   0x34    Data base        (load address of RW area)
// 14-15 0x38    Reserved
// 16-31 0x40    Debug init instructions (ARM code run before entry)
// ----------------------------------------------------------------------------

static constexpr uint32_t AIF_HEADER_SIZE  = 0x80;   // 128 bytes / 32 words
static constexpr uint32_t AIF_WORD_RO_SIZE = 5;
static constexpr uint32_t AIF_WORD_RW_SIZE = 6;
static constexpr uint32_t AIF_WORD_IMAGE_BASE = 10;

// ---- helpers ----------------------------------------------------------------

static std::string file_error(const std::string& context, const std::string& path)
{
    return std::format("{}: {} -- {}", context, path, strerror(errno));
}

static uint32_t read_u32_be(const uint8_t* p)
{
    return (static_cast<uint32_t>(p[0]) << 24)
         | (static_cast<uint32_t>(p[1]) << 16)
         | (static_cast<uint32_t>(p[2]) <<  8)
         |  static_cast<uint32_t>(p[3]);
}

// ---- public -----------------------------------------------------------------

uint32_t read_aif(const std::string& path, Memory& memory, LabelMap& labels)
{
    // --- open ----------------------------------------------------------------
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        throw std::runtime_error(file_error("Cannot open AIF file", path));

    // --- read header ---------------------------------------------------------
    uint8_t header[AIF_HEADER_SIZE];
    if (fread(header, 1, AIF_HEADER_SIZE, f) != AIF_HEADER_SIZE) {
        fclose(f);
        throw std::runtime_error(std::format("AIF header too short: {}", path));
    }

    const uint32_t ro_size   = read_u32_be(header + AIF_WORD_RO_SIZE   * 4);
    const uint32_t rw_size   = read_u32_be(header + AIF_WORD_RW_SIZE   * 4);
    const uint32_t load_base = read_u32_be(header + AIF_WORD_IMAGE_BASE * 4);
    uint32_t total_size = ro_size + rw_size;

    if (total_size == 0)
        throw std::runtime_error(std::format("AIF header reports zero RO size: {}", path));

    // --- load image into Memory ----------------------------------------------
    std::vector<uint8_t> image(total_size);
    if (fread(image.data(), 1, total_size, f) != total_size) {
        fclose(f);
        throw std::runtime_error(file_error("Failed to read AIF image data", path));
    }

    memory.block_at(0x00000000)->write_block(load_base, image);

    // --- symbol table --------------------------------------------------------
    // The AIF symbol table follows the image data (and any RW / BSS areas).
    // Its exact location is derived from the debug area fields in the header;
    // that parsing will be added here as a second pass once the header fields
    // are confirmed against the Newton AIF binary.
    //
    // TODO: read debug size, seek to symbol table, iterate entries:
    //
    //   struct AIFSymbolEntry {
    //       uint32_t flags_and_type;   // upper 8 bits: flags; lower 24: type
    //       uint32_t value;            // address (for code/data) or constant
    //       uint32_t name_offset;      // byte offset into the string table
    //   };
    //
    // For each entry:
    //   LabelFlags f = LabelFlags::FromAIF;
    //   if (entry.flags & AIF_SYM_GLOBAL) f = f | LabelFlags::Global;
    //   labels.emplace(entry.value, name_from_string_table, f);

    (void)labels;   // suppress unused-parameter warning until TODO is filled in

    fclose(f);

    return load_base + total_size;
}
