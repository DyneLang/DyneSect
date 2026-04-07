#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// MemoryBlock
//
// Represents a contiguous range of the 32-bit ARM address space.
//
// By default no memory is allocated: every read returns 0xba (ARM's
// unofficial "uninitialised memory" fill byte). Call allocate() to back the
// block with real storage, then load data with write_block().
//
// All multi-byte reads are big-endian (MSB first). Reads must be naturally
// aligned -- misaligned access throws std::logic_error.
// ----------------------------------------------------------------------------
class MemoryBlock
{
public:
    MemoryBlock(uint32_t base, uint32_t size);

    uint32_t base() const { return m_base; }
    uint32_t size() const { return m_size; }
    uint32_t end()  const { return m_base + m_size; }

    // Returns true if the byte range [address, address+access_size) lies
    // entirely within this block.
    bool covers(uint32_t address, uint32_t access_size) const;

    // Allocate backing storage. Safe to call more than once; subsequent calls
    // are ignored. Storage is zero-initialised.
    void allocate();
    bool is_allocated() const { return m_data.has_value(); }

    // Copy bytes into allocated storage at address (must be within the block).
    // Throws std::logic_error if the block has not been allocated, or if the
    // range extends beyond the block boundary.
    void write_block(uint32_t address, std::span<const uint8_t> data);

    // Read access -- big-endian, naturally aligned.
    uint8_t  read_byte (uint32_t address) const;
    uint16_t read_short(uint32_t address) const;
    uint32_t read_word (uint32_t address) const;

private:
    uint32_t                          m_base;
    uint32_t                          m_size;
    std::optional<std::vector<uint8_t>> m_data;   // absent → returns 0xba

    static constexpr uint8_t UNALLOCATED_FILL = 0xba;

    void check_alignment(uint32_t address, uint32_t access_size) const;
    void check_range    (uint32_t address, uint32_t access_size) const;
    uint8_t byte_at(uint32_t address) const;
};

// ----------------------------------------------------------------------------
// Memory
//
// Models the full 32-bit ARM address space as a sparse collection of
// MemoryBlocks. Blocks must not overlap. Reads are dispatched to the
// appropriate block; access to uncovered addresses throws std::out_of_range.
// ----------------------------------------------------------------------------
class Memory
{
public:
    // Register a block. Throws std::logic_error if it overlaps an existing block.
    void add_block(std::shared_ptr<MemoryBlock> block);

    // Convenience: create, allocate, and register a block in one call.
    MemoryBlock& add_allocated_block(uint32_t base, uint32_t size);

    // Convenience: create and register an unallocated (0xba) block.
    MemoryBlock& add_unallocated_block(uint32_t base, uint32_t size);

    // Find the block that covers address, or nullptr.
    MemoryBlock* block_at(uint32_t address) const;

    // Read access -- big-endian, naturally aligned.
    // Throws std::out_of_range if no block covers the address.
    uint8_t  read_byte (uint32_t address) const;
    uint16_t read_short(uint32_t address) const;
    uint32_t read_word (uint32_t address) const;

private:
    // Keyed by base address; blocks are non-overlapping so this is sufficient
    // for O(log n) lookup via upper_bound.
    std::map<uint32_t, std::shared_ptr<MemoryBlock>> m_blocks;

    MemoryBlock& require_block(uint32_t address, uint32_t access_size) const;
};
