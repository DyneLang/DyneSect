#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

// Forward declaration -- Type will be defined in its own header later.
class Type;

// ----------------------------------------------------------------------------
// LabelFlags
//
// Bits 0–3 describe the symbol's segment, derived from the AIF flag byte.
// The AIF flag byte encodes: bit 0 = Global (always set for exported symbols);
// bits 1–2 = segment type as a 2-bit field (see InRO/InRW/InZI below).
//
// Bits 8–11 record where the label came from.
// ----------------------------------------------------------------------------
enum class LabelFlags : uint32_t
{
    None        = 0,

    // Segment membership -- mutually exclusive, derived from AIF bits 1-2.
    // Absolute linker symbols (AIF type 00) carry none of these flags.
    Global      = 1 << 0,   // Exported / globally visible (AIF bit 0, always set)
    InRO        = 1 << 1,   // Read-Only segment: ROM code and constants (AIF type 01)
    InRW        = 1 << 2,   // Read-Write segment: initialized globals (AIF type 10)
    InZI        = 1 << 3,   // Zero-Init segment: BSS / zeroed at boot (AIF type 11)

    // Source of the label
    FromAIF     = 1 << 8,   // Read from the AIF symbol table
    FromREx     = 1 << 9,   // Read from the REx image
    Manual      = 1 << 10,  // Added manually (e.g. from a JSON override file)
    Generated   = 1 << 11,  // Synthesised automatically (e.g. branch target)
};

inline LabelFlags operator|(LabelFlags a, LabelFlags b)
{
    return static_cast<LabelFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline LabelFlags operator&(LabelFlags a, LabelFlags b)
{
    return static_cast<LabelFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool has_flag(LabelFlags flags, LabelFlags test)
{
    return (flags & test) != LabelFlags::None;
}

// ----------------------------------------------------------------------------
// Label
// ----------------------------------------------------------------------------
class Label
{
public:
    Label(uint32_t address, std::string name, LabelFlags flags = LabelFlags::None);

    uint32_t           address() const { return m_address; }
    const std::string& name()    const { return m_name; }
    LabelFlags         flags()   const { return m_flags; }

    // Type is optional -- null until resolved.
    const Type*               type() const { return m_type.get(); }
    void set_type(std::shared_ptr<Type> type) { m_type = std::move(type); }

    bool has_flag(LabelFlags f) const { return ::has_flag(m_flags, f); }
    void set_flag(LabelFlags f) { m_flags = m_flags | f; }

    // Cross-reference placeholders -- populated later as the analysis deepens.
    // These will track which addresses call, read, or write this label.
    // std::vector<uint32_t> callers;
    // std::vector<uint32_t> readers;
    // std::vector<uint32_t> writers;

private:
    uint32_t              m_address;
    std::string           m_name;
    LabelFlags            m_flags;
    std::shared_ptr<Type> m_type;   // null until type information is resolved
};

// ----------------------------------------------------------------------------
// LabelMap
// ----------------------------------------------------------------------------
class LabelMap
{
public:
    // Add a label. If a label at that address already exists it is replaced.
    void add(std::shared_ptr<Label> label);

    // Convenience: construct and add in one call.
    Label& emplace(uint32_t address, std::string name,
                   LabelFlags flags = LabelFlags::None);

    // Look up by address. Returns nullptr if not found.
    Label* find(uint32_t address) const;

    // True if there is a label at this address.
    bool contains(uint32_t address) const;

    // Iteration over the full map in address order.
    using Map = std::map<uint32_t, std::shared_ptr<Label>>;
    Map::const_iterator begin() const { return m_labels.begin(); }
    Map::const_iterator end()   const { return m_labels.end(); }
    std::size_t size() const { return m_labels.size(); }

private:
    Map m_labels;
};
