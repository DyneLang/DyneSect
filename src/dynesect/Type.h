#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// Type  (abstract base)
//
// All C/C++ types parsed from include files derive from this class.
// Types are heap-allocated and shared via shared_ptr so that the same type
// object (e.g. "int") can be referenced from many fields, parameters, and
// labels without duplication.
//
// size() returns the sizeof value in bytes for the target (Newton ARM32).
// It returns 0 for types whose size is not yet known (incomplete forward
// declarations) or that have no meaningful size (bare function types).
// ----------------------------------------------------------------------------
class Type
{
public:
    virtual ~Type() = default;

    // Declared identifier; empty for anonymous structs/enums/unions.
    const std::string& name() const { return m_name; }

    // sizeof in bytes on the target platform; 0 = unknown or not applicable.
    virtual uint32_t size() const = 0;

protected:
    explicit Type(std::string name) : m_name(std::move(name)) {}

private:
    std::string m_name;
};


// ----------------------------------------------------------------------------
// PrimitiveType  —  void, char, int, long, float, double, and unsigned variants
// ----------------------------------------------------------------------------
class PrimitiveType : public Type
{
public:
    enum class Kind {
        Void,
        Char,   Short,   Int,   Long,
        UChar,  UShort,  UInt,  ULong,
        Float,  Double
    };

    PrimitiveType(Kind kind, uint32_t size);

    Kind     kind() const { return m_kind; }
    uint32_t size() const override { return m_size; }

private:
    Kind     m_kind;
    uint32_t m_size;
};


// ----------------------------------------------------------------------------
// PointerType  —  T*
//
// On Newton ARM32 all pointers are 4 bytes wide.
// ----------------------------------------------------------------------------
class PointerType : public Type
{
public:
    explicit PointerType(std::shared_ptr<Type> pointee);

    const Type*           pointee()        const { return m_pointee.get(); }
    std::shared_ptr<Type> pointee_shared() const { return m_pointee; }
    uint32_t              size()           const override { return 4; }

private:
    std::shared_ptr<Type> m_pointee;
};


// ----------------------------------------------------------------------------
// ArrayType  —  T[N]
//
// count == 0 means an unknown or flexible-array-member size.
// ----------------------------------------------------------------------------
class ArrayType : public Type
{
public:
    ArrayType(std::shared_ptr<Type> element, uint32_t count);

    const Type*           element_type() const { return m_element.get(); }
    std::shared_ptr<Type> element_shared() const { return m_element; }
    uint32_t              count()        const { return m_count; }
    uint32_t              size()         const override;

private:
    std::shared_ptr<Type> m_element;
    uint32_t              m_count;
};


// ----------------------------------------------------------------------------
// EnumType  —  enum { A = 0, B = 1, ... }
//
// The underlying type is usually int; it is stored so that size() can
// delegate without hard-coding.
// ----------------------------------------------------------------------------
class EnumType : public Type
{
public:
    struct Enumerator {
        std::string name;
        int32_t     value;
    };

    EnumType(std::string name, std::shared_ptr<Type> underlying);

    void add(std::string name, int32_t value);

    const std::vector<Enumerator>& enumerators() const { return m_enumerators; }
    const Type*                    underlying()   const { return m_underlying.get(); }
    uint32_t                       size()         const override;

private:
    std::shared_ptr<Type>   m_underlying;
    std::vector<Enumerator> m_enumerators;
};


// ----------------------------------------------------------------------------
// FunctionType  —  return_type ( param_types... )
//
// Represents the signature of a function or function pointer.
// A bare FunctionType has no sizeof (returns 0); wrap it in a PointerType
// to model a function-pointer variable (sizeof == 4).
// ----------------------------------------------------------------------------
class FunctionType : public Type
{
public:
    struct Param {
        std::string           name;  // may be empty in declarations
        std::shared_ptr<Type> type;
    };

    explicit FunctionType(std::shared_ptr<Type> return_type);

    void add_param(std::string name, std::shared_ptr<Type> type);

    const Type*              return_type() const { return m_return_type.get(); }
    const std::vector<Param>& params()    const { return m_params; }
    uint32_t                  size()      const override { return 0; }

private:
    std::shared_ptr<Type>  m_return_type;
    std::vector<Param>     m_params;
};


// ----------------------------------------------------------------------------
// RecordType  —  struct / class / union
//
// Fields, methods, and base classes are added incrementally as the parser
// reads the definition.  The record is considered incomplete (is_complete()
// == false) until set_size() is called; incomplete records can still be
// pointed to.
// ----------------------------------------------------------------------------
class RecordType : public Type
{
public:
    enum class Kind   { Struct, Class, Union };
    enum class Access { Public, Protected, Private };

    struct Field {
        std::string           name;
        std::shared_ptr<Type> type;
        uint32_t              offset;      // byte offset from record start
        int                   bit_offset;  // -1 if not a bitfield
        int                   bit_width;   // 0  if not a bitfield
    };

    struct Method {
        std::string                   name;
        std::shared_ptr<FunctionType> type;
        Access                        access;
        bool                          is_virtual;
        bool                          is_static;
    };

    struct BaseClass {
        std::shared_ptr<RecordType> type;
        Access                      access;
        uint32_t                    offset;  // byte offset of base subobject
    };

    RecordType(std::string name, Kind kind);

    // Called once the full definition has been parsed.
    void set_size(uint32_t size);

    void add_field (Field     field);
    void add_method(Method    method);
    void add_base  (BaseClass base);

    Kind                          kind()      const { return m_kind; }
    uint32_t                      size()      const override { return m_size; }
    bool                          is_complete() const { return m_complete; }
    const std::vector<Field>&     fields()    const { return m_fields; }
    const std::vector<Method>&    methods()   const { return m_methods; }
    const std::vector<BaseClass>& bases()     const { return m_bases; }

private:
    Kind                   m_kind;
    uint32_t               m_size     = 0;
    bool                   m_complete = false;
    std::vector<Field>     m_fields;
    std::vector<Method>    m_methods;
    std::vector<BaseClass> m_bases;
};


// ----------------------------------------------------------------------------
// TypedefType  —  typedef <underlying> <alias>
//
// name() (inherited) is the alias identifier.
// underlying() is the type it expands to; follow the chain to reach the
// concrete type.
// ----------------------------------------------------------------------------
class TypedefType : public Type
{
public:
    TypedefType(std::string alias, std::shared_ptr<Type> underlying);

    const Type*           underlying()        const { return m_underlying.get(); }
    std::shared_ptr<Type> underlying_shared() const { return m_underlying; }
    uint32_t              size()              const override;

private:
    std::shared_ptr<Type> m_underlying;
};


// ----------------------------------------------------------------------------
// TypeRegistry  —  the central store for all known types
//
// The constructor pre-populates one PrimitiveType instance per Kind so that
// callers can always get a shared pointer to (e.g.) "int" without having to
// create one themselves.
//
// resolve() walks typedef chains and returns the first non-typedef type,
// which is useful when you want the concrete layout rather than the alias.
// ----------------------------------------------------------------------------
class TypeRegistry
{
public:
    TypeRegistry();

    // Register a named type.  Replaces any previous registration for that name.
    void add(std::shared_ptr<Type> type);

    // Look up by name.  Returns nullptr if not found.
    Type* find(const std::string& name) const;

    // Walk typedef chains until a non-typedef type is reached.
    // Returns nullptr if the name is not registered.
    const Type* resolve(const std::string& name) const;

    // Pre-populated singleton for each primitive kind.
    PrimitiveType* primitive(PrimitiveType::Kind kind) const;

private:
    std::map<std::string, std::shared_ptr<Type>>             m_types;
    std::map<PrimitiveType::Kind, std::shared_ptr<PrimitiveType>> m_primitives;
};
