#include "AIFReader.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>

// ----------------------------------------------------------------------------
// AIF header layout (ARM DUI 0041)
//
// 32 words (128 bytes) at the start of the file. All fields are big-endian
// 32-bit words, consistent with the Newton OS address space.
//
// Word  Offset  Value       Field
//   0   0x00    0xe1a00000  BL <decompress>  or NOP (0xE1A00000)
//   1   0x04    0xe1a00000  BL <self-relocate> or NOP
//   2   0x08    0xeb00000c  BL <zero-init>   or NOP
//   3   0x0C    0x00000000  BL <entry>       or NOP  (branch to entry point)
//   4   0x10    0xef000011  SWI OS_Exit      (0xEF000011)
//   5   0x14    0x0071a95c  RO size          (read-only area, bytes)
//   6   0x18    0x000052f0  RW size          (read-write area, bytes)
//   7   0x1C    0x001cadf0  Debug size       (debug area, bytes; 0 if none)
//   8   0x20    0x00002324  Zero-init size   (BSS, bytes)
//   9   0x24    0x01cadf01  Debug type       (0 = none, 1 = ASD, 2 = DWARF …)
//  10   0x28    0x00000000  Image base       (load address)
//  11   0x2C    0x00000000  Work space       (minimum stack, bytes)
//  12   0x30    0x00000120  Address mode     (26 or 32)
//  13   0x34    0x0c100800  Data base        (load address of RW area)
// 14-15 0x38                Reserved
// 16-31 0x40                Debug init instructions (ARM code run before entry)
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

static uint32_t read_u32_be(FILE* f)
{
    uint8_t buf[4];
    fread(buf, 1, 4, f);
    return read_u32_be(buf);
}

static std::string read_pascal_string(FILE* f, long string_table_offset, uint32_t name_offset)
{
    long saved = ftell(f);
    fseek(f, string_table_offset + name_offset, SEEK_SET);
    uint8_t len;
    fread(&len, 1, 1, f);
    char buf[256];
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fseek(f, saved, SEEK_SET);
    return std::string(buf);
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
    auto image = std::make_unique<uint8_t[]>(total_size);
    if (fread(image.get(), 1, total_size, f) != total_size) {
        fclose(f);
        throw std::runtime_error(file_error("Failed to read AIF image data", path));
    }

    memory.block_at(0x00000000)->write_block(load_base, std::span(image.get(), total_size));

    // Print the header for evaluation:
    // for (int i=0; i<32; i++) {
    //     printf("%3d: 0x%08x\n", i, read_u32_be(header + i * 4));
    // }

    // --- symbol table --------------------------------------------------------
    // The symbol list starts immediately after the RO+RW image data in the
    // file (i.e. at file offset AIF_HEADER_SIZE + ro_size + rw_size).
    //
    // Layout of the symbol list block:
    //   words 0..7   : 8 reserved/unknown words (skipped)
    //   word  8      : number of symbol entries
    //   word  9+i*2  : (flags[7:0] << 24) | name_offset[23:0]  -- entry i
    //   word 10+i*2  : value                                    -- entry i
    //
    // The string table follows immediately after all entries.
    // Names are Pascal-style: one length byte then that many characters.
    //
    // Observed flag values (exact meaning still under investigation):
    //   0x01 : possible constants / linker-internal symbols -- value may not
    //          be a ROM address; printed to stdout for now, not added to labels
    //   0x03 : functions and globals in ROM (0x00000000..0x00800000) and in
    //          the jump table area (0x01a00000..0x01c20000)
    //   0x05 : globals in RAM starting around 0x0c100800
    //   0x07 : more globals at higher RAM addresses (0x0c105af0 and up)

    const long sym_list_offset = AIF_HEADER_SIZE + ro_size + rw_size;
    fseek(f, sym_list_offset + 8 * 4, SEEK_SET);   // skip 8 reserved words
    const uint32_t num_symbols = read_u32_be(f);

    // String table starts right after the entry array.
    const long string_table_offset = sym_list_offset + 9 * 4 + num_symbols * 8;

    for (uint32_t i = 0; i < num_symbols; i++) {
        const uint32_t packed      = read_u32_be(f);
        const uint32_t value       = read_u32_be(f);
        const uint8_t  flags       = static_cast<uint8_t>(packed >> 24);
        const uint32_t name_offset = packed & 0x00ffffff;

        const std::string name = read_pascal_string(f, string_table_offset, name_offset);

        // Bits 1-2 of the AIF flag byte encode the Norcroft segment type.
        LabelFlags lflags = LabelFlags::FromAIF | LabelFlags::Global;
        switch ((flags >> 1) & 0x03) {
            case 0x00:
                // Absolute linker symbol: size, limit, or constant value.
                // Not a ROM address -- print for inspection until confirmed.
                std::cout << std::format("abs-sym  0x{:02x}  0x{:08x}  {}\n", flags, value, name);
                continue;
            case 0x01: lflags = lflags | LabelFlags::InRO; break;   // ROM code
            case 0x02: lflags = lflags | LabelFlags::InRW; break;   // initialized globals
            case 0x03: lflags = lflags | LabelFlags::InZI; break;   // zero-init globals
        }

        labels.emplace(value, name, lflags);
    }

    fclose(f);

    return load_base + total_size;
}

// Compiler related symbols
// 0x01  0x0071fc4c  ROM$$Size
// 0x01  0x0071a95c  Image$$RO$$Limit
// 0x01  0x0c105af0  Image$$ZI$$Base  // zero initialised RAM?!
// 0x01  0x0c107e14  _end

// A hand full of MMU related constants
// 0x01  0x00000020  kEntriesPerSlot

// Almost all symbols:
// 0x03  0x00216d4c  CurvePts__FP5CurvePlP6FPointl

// Symbols in system RAM (0x0c*, is everything there 0x05 or 0x07?)
// Many g* globals, also static class variables
// 0x05  0x0c105a0c, GSB Frequency Table
// 0x05  0x0c104f7c  gWRecHeap

// 0x07 and 0x05 seem a bit random?!
// 0x05  0x0c10555c  gLastFindOffsetCacheEntry
// 0x07  0x0c107bf4  gFindOffsetCache

// All 0x05 labels are above 0x0c000000, so this is probably where the BSS segment is mapped
// All 0x07 labels are greater than this value, so probably inside the ZI segment
// R/W segment is only 0x000052f0 bytes though (52f0 vs. 5af0, 800 bytes diff!)
// R/W segment start is at 0x0c100800 (note the '8' in the middle!)
// ZI segment is offset by exactly R/W size
// 0x07  0x0c105af0  C$$ddtorvec$$Base

// Memory map (virtual)
// https://github.com/pguyot/Einstein/wiki/Mem-Map-Virtual

// Memory Mapped I/O
// https://github.com/pguyot/Einstein/wiki/Mem-MapIO
