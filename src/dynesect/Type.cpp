#include "Type.h"

#include <stdexcept>

// ----------------------------------------------------------------------------
// PrimitiveType
// ----------------------------------------------------------------------------

PrimitiveType::PrimitiveType(Kind kind, uint32_t size)
    : Type("")          // primitives use their kind, not a string name
    , m_kind(kind)
    , m_size(size)
{
}


// ----------------------------------------------------------------------------
// PointerType
// ----------------------------------------------------------------------------

PointerType::PointerType(std::shared_ptr<Type> pointee)
    : Type("")
    , m_pointee(std::move(pointee))
{
}


// ----------------------------------------------------------------------------
// ArrayType
// ----------------------------------------------------------------------------

ArrayType::ArrayType(std::shared_ptr<Type> element, uint32_t count)
    : Type("")
    , m_element(std::move(element))
    , m_count(count)
{
}

uint32_t ArrayType::size() const
{
    if (m_count == 0 || !m_element)
        return 0;
    return m_element->size() * m_count;
}


// ----------------------------------------------------------------------------
// EnumType
// ----------------------------------------------------------------------------

EnumType::EnumType(std::string name, std::shared_ptr<Type> underlying)
    : Type(std::move(name))
    , m_underlying(std::move(underlying))
{
}

void EnumType::add(std::string name, int32_t value)
{
    m_enumerators.push_back({ std::move(name), value });
}

uint32_t EnumType::size() const
{
    return m_underlying ? m_underlying->size() : 0;
}


// ----------------------------------------------------------------------------
// FunctionType
// ----------------------------------------------------------------------------

FunctionType::FunctionType(std::shared_ptr<Type> return_type)
    : Type("")
    , m_return_type(std::move(return_type))
{
}

void FunctionType::add_param(std::string name, std::shared_ptr<Type> type)
{
    m_params.push_back({ std::move(name), std::move(type) });
}


// ----------------------------------------------------------------------------
// RecordType
// ----------------------------------------------------------------------------

RecordType::RecordType(std::string name, Kind kind)
    : Type(std::move(name))
    , m_kind(kind)
{
}

void RecordType::set_size(uint32_t size)
{
    m_size     = size;
    m_complete = true;
}

void RecordType::add_field(Field field)
{
    m_fields.push_back(std::move(field));
}

void RecordType::add_method(Method method)
{
    m_methods.push_back(std::move(method));
}

void RecordType::add_base(BaseClass base)
{
    m_bases.push_back(std::move(base));
}


// ----------------------------------------------------------------------------
// TypedefType
// ----------------------------------------------------------------------------

TypedefType::TypedefType(std::string alias, std::shared_ptr<Type> underlying)
    : Type(std::move(alias))
    , m_underlying(std::move(underlying))
{
}

uint32_t TypedefType::size() const
{
    return m_underlying ? m_underlying->size() : 0;
}


// ----------------------------------------------------------------------------
// TypeRegistry
// ----------------------------------------------------------------------------

TypeRegistry::TypeRegistry()
{
    // Pre-populate one PrimitiveType instance per Kind.
    // Sizes reflect Newton's ARM32 ABI (same as most 32-bit ARM ABIs).
    auto make = [&](PrimitiveType::Kind k, uint32_t sz) {
        auto p = std::make_shared<PrimitiveType>(k, sz);
        m_primitives[k] = p;
        // Primitives are not inserted into m_types by name because they have
        // no reliable single string identifier (e.g. "unsigned int" vs "uint").
        // The caller registers typedefs that point at these as needed.
    };

    make(PrimitiveType::Kind::Void,   0);
    make(PrimitiveType::Kind::Char,   1);
    make(PrimitiveType::Kind::Short,  2);
    make(PrimitiveType::Kind::Int,    4);
    make(PrimitiveType::Kind::Long,   4);
    make(PrimitiveType::Kind::UChar,  1);
    make(PrimitiveType::Kind::UShort, 2);
    make(PrimitiveType::Kind::UInt,   4);
    make(PrimitiveType::Kind::ULong,  4);
    make(PrimitiveType::Kind::Float,  4);
    make(PrimitiveType::Kind::Double, 8);
}

void TypeRegistry::add(std::shared_ptr<Type> type)
{
    if (!type)
        throw std::invalid_argument("TypeRegistry::add: null type");
    if (type->name().empty())
        throw std::invalid_argument("TypeRegistry::add: type has no name");
    m_types[type->name()] = std::move(type);
}

Type* TypeRegistry::find(const std::string& name) const
{
    auto it = m_types.find(name);
    return (it != m_types.end()) ? it->second.get() : nullptr;
}

const Type* TypeRegistry::resolve(const std::string& name) const
{
    const Type* t = find(name);
    // Walk typedef chains.
    while (t) {
        const auto* td = dynamic_cast<const TypedefType*>(t);
        if (!td)
            break;
        t = td->underlying();
    }
    return t;
}

PrimitiveType* TypeRegistry::primitive(PrimitiveType::Kind kind) const
{
    auto it = m_primitives.find(kind);
    return (it != m_primitives.end()) ? it->second.get() : nullptr;
}
