#include "Memory.h"

#include <algorithm>
#include <format>

// ============================================================================
// MemoryBlock
// ============================================================================

MemoryBlock::MemoryBlock(uint32_t base, uint32_t size)
    : m_base(base)
    , m_size(size)
{
}

bool MemoryBlock::covers(uint32_t address, uint32_t access_size) const
{
    return address >= m_base && (address + access_size) <= (m_base + m_size);
}

void MemoryBlock::allocate()
{
    if (!m_data)
        m_data.emplace(m_size, uint8_t{0});
}

void MemoryBlock::write_block(uint32_t address, std::span<const uint8_t> data)
{
    if (!m_data)
        throw std::logic_error("MemoryBlock::write_block: block is not allocated");
    if (!covers(address, static_cast<uint32_t>(data.size())))
        throw std::logic_error(std::format(
            "MemoryBlock::write_block: range [{:#010x}, {:#010x}) exceeds block [{:#010x}, {:#010x})",
            address, address + data.size(), m_base, m_base + m_size));

    std::copy(data.begin(), data.end(), m_data->begin() + (address - m_base));
}

// --- private helpers --------------------------------------------------------

void MemoryBlock::check_alignment(uint32_t address, uint32_t access_size) const
{
    if (address & (access_size - 1))
        throw std::logic_error(std::format(
            "MemoryBlock: misaligned {}-byte access at {:#010x}", access_size, address));
}

void MemoryBlock::check_range(uint32_t address, uint32_t access_size) const
{
    if (!covers(address, access_size))
        throw std::out_of_range(std::format(
            "MemoryBlock: {}-byte access at {:#010x} is outside block [{:#010x}, {:#010x})",
            access_size, address, m_base, m_base + m_size));
}

uint8_t MemoryBlock::byte_at(uint32_t address) const
{
    if (!m_data)
        return UNALLOCATED_FILL;
    return (*m_data)[address - m_base];
}

// --- read access ------------------------------------------------------------

uint8_t MemoryBlock::read_byte(uint32_t address) const
{
    check_range(address, 1);
    return byte_at(address);
}

uint16_t MemoryBlock::read_short(uint32_t address) const
{
    check_alignment(address, 2);
    check_range(address, 2);
    return static_cast<uint16_t>(byte_at(address)     << 8)
         | static_cast<uint16_t>(byte_at(address + 1));
}

uint32_t MemoryBlock::read_word(uint32_t address) const
{
    check_alignment(address, 4);
    check_range(address, 4);
    return static_cast<uint32_t>(byte_at(address)     << 24)
         | static_cast<uint32_t>(byte_at(address + 1) << 16)
         | static_cast<uint32_t>(byte_at(address + 2) <<  8)
         | static_cast<uint32_t>(byte_at(address + 3));
}

// ============================================================================
// Memory
// ============================================================================

void Memory::add_block(std::shared_ptr<MemoryBlock> block)
{
    // Check for overlap with existing blocks.
    auto it = m_blocks.upper_bound(block->base());
    if (it != m_blocks.begin()) {
        auto prev = std::prev(it);
        if (prev->second->end() > block->base())
            throw std::logic_error(std::format(
                "Memory::add_block: new block [{:#010x}, {:#010x}) overlaps existing block [{:#010x}, {:#010x})",
                block->base(), block->end(),
                prev->second->base(), prev->second->end()));
    }
    if (it != m_blocks.end() && block->end() > it->second->base())
        throw std::logic_error(std::format(
            "Memory::add_block: new block [{:#010x}, {:#010x}) overlaps existing block [{:#010x}, {:#010x})",
            block->base(), block->end(),
            it->second->base(), it->second->end()));

    m_blocks[block->base()] = std::move(block);
}

MemoryBlock& Memory::add_allocated_block(uint32_t base, uint32_t size)
{
    auto block = std::make_shared<MemoryBlock>(base, size);
    block->allocate();
    MemoryBlock& ref = *block;
    add_block(std::move(block));
    return ref;
}

MemoryBlock& Memory::add_unallocated_block(uint32_t base, uint32_t size)
{
    auto block = std::make_shared<MemoryBlock>(base, size);
    MemoryBlock& ref = *block;
    add_block(std::move(block));
    return ref;
}

MemoryBlock* Memory::block_at(uint32_t address) const
{
    if (m_blocks.empty())
        return nullptr;

    // Find the first block whose base is strictly greater than address,
    // then step back to get the candidate.
    auto it = m_blocks.upper_bound(address);
    if (it == m_blocks.begin())
        return nullptr;

    --it;
    MemoryBlock* candidate = it->second.get();
    return candidate->covers(address, 1) ? candidate : nullptr;
}

MemoryBlock& Memory::require_block(uint32_t address, uint32_t access_size) const
{
    if (m_blocks.empty())
        goto not_found;
    {
        auto it = m_blocks.upper_bound(address);
        if (it != m_blocks.begin()) {
            --it;
            if (it->second->covers(address, access_size))
                return *it->second;
        }
    }
not_found:
    throw std::out_of_range(std::format(
        "Memory: {}-byte access at {:#010x} is not covered by any block",
        access_size, address));
}

uint8_t Memory::read_byte(uint32_t address) const
{
    return require_block(address, 1).read_byte(address);
}

uint16_t Memory::read_short(uint32_t address) const
{
    return require_block(address, 2).read_short(address);
}

uint32_t Memory::read_word(uint32_t address) const
{
    return require_block(address, 4).read_word(address);
}
