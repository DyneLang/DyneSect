#include "Label.h"

// ----------------------------------------------------------------------------
// Label
// ----------------------------------------------------------------------------

Label::Label(uint32_t address, std::string name, LabelFlags flags)
    : m_address(address)
    , m_name(std::move(name))
    , m_flags(flags)
{
}

// ----------------------------------------------------------------------------
// LabelMap
// ----------------------------------------------------------------------------

void LabelMap::add(std::shared_ptr<Label> label)
{
    m_labels[label->address()] = std::move(label);
}

Label& LabelMap::emplace(uint32_t address, std::string name, LabelFlags flags)
{
    auto lbl = std::make_shared<Label>(address, std::move(name), flags);
    Label& ref = *lbl;
    m_labels[address] = std::move(lbl);
    return ref;
}

Label* LabelMap::find(uint32_t address) const
{
    auto it = m_labels.find(address);
    return it != m_labels.end() ? it->second.get() : nullptr;
}

bool LabelMap::contains(uint32_t address) const
{
    return m_labels.contains(address);
}
