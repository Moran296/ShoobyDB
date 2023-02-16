#ifndef _SHOOBY_DB_H_
#define _SHOOBY_DB_H_

#include "shooby_utilities.h"
#include <cassert>
#include <cstring>

// ================== META DATA CLASS =================

struct MetaData
{
    template <Arithmetic T>
    consteval MetaData(const char *n, T num_default) : size(sizeof(T)), name(n), default_val(num_default) {}

    consteval MetaData(const char *n, size_t s, const char *def_str) : size(s), name(n), default_val(def_str) {}

    template <class T>
    consteval MetaData(const char *n, const T *def_blob, size_t s = sizeof(T)) : size(s), name(n), default_val((const void *)def_blob)
    {
        assert(sizeof(T) == size);
    }

    const size_t size;
    const char *name;
    const value_variant_t default_val;
};

template <EnumMetaMap E>
class ShoobyDB
{
public:
    // reset buffer to default!
    static void Reset();

    template <class T>
    static T Get(E::enum_type e);

    template <class T>
    static void Set(E::enum_type e, const T &t);

private:
    static inline constinit uint8_t DATA_BUFFER[E::required_buffer_size()]{};

    static size_t get_offset(E::enum_type e);
    static const char *get_name(E::enum_type e) { return E::META_MAP[e].name; }
    static size_t get_size(E::enum_type e) { return E::META_MAP[e].size; }
};

#include "shooby_db_inl.hpp"

#endif