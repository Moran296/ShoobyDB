#ifndef _SHOOBY_DB_H_
#define _SHOOBY_DB_H_

#include "shooby_utilities.h"
#include "shooby_config.h"
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
        SHOOBY_ASSERT(sizeof(T) == size);
    }

    const size_t size;
    const char *name;
    const value_variant_t default_val;
};

template <EnumMetaMap E>
class ShoobyDB
{
public:
    typedef void (*observer_f)(E::enum_type, const void *, size_t, void *);

    static void Init();
    static void Reset();

    template <class T>
    static T Get(E::enum_type e);

    template <class T>
    static bool Set(E::enum_type e, const T &t);

    template <class Visitor>
    static void VisitRaw(Visitor &&visitor);

    static void SetObserver(observer_f f, void *user_data = nullptr);

    static const char *get_name(E::enum_type e) { return E::META_MAP[e].name; }
    static size_t get_size(E::enum_type e) { return E::META_MAP[e].size; }

private:
    // CTORS
    ShoobyDB(const ShoobyDB &) = delete;
    ShoobyDB &operator=(const ShoobyDB &) = delete;
    ShoobyDB(ShoobyDB &&) = delete;
    ShoobyDB &operator=(ShoobyDB &&) = delete;

    // DATA RELATED
    static inline constexpr size_t required_data_buffer_size = required_buffer_size<E>();
    static inline constinit uint8_t DATA_BUFFER[required_data_buffer_size]{};
    static size_t get_offset(E::enum_type e);

    template <class T>
    static bool set_if_changed(void *dst, const T &src, size_t size);

    // INITIALIZATION RELATED
    static inline bool s_is_initialized = false;

    // OBSERVER CALLBACK
    static inline observer_f observer = nullptr;
    static inline void *observer_user_data = nullptr;

    // SYNCHRONIZATION
    static SHOOBY_MUTEX_TYPE s_mutex;

    // BACKEND
};

#include "shooby_db_inl.hpp"

#endif