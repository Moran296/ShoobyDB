#ifndef __SHOOBY_META_MAP_H__
#define __SHOOBY_META_MAP_H__

#include "shooby_db.h"

#define DEFINE_SHOOBY_META_MAP(NAME) \
    struct NAME                      \
    {                                \
        static constexpr const char *name = #NAME;

#define SHOOBY_ENUMS(...) \
    enum enum_type        \
    {                     \
        __VA_ARGS__,      \
        NUM               \
    };

#define SHOOBY_META_MAP_START                                 \
    static inline constexpr MetaData META_MAP[enum_type::NUM] \
    {

#define META_MAP_INTEGRAL(ENUM, TYPE, DEFAULT) \
    [ENUM] = {#ENUM, TYPE(DEFAULT)},
#define META_MAP_STRING(ENUM, DEFAULT, SIZE) \
    [ENUM] = {#ENUM, SIZE, DEFAULT},
#define META_MAP_BLOB(ENUM, TYPE, DEFAULT_INSTANCE) \
    [ENUM] = {#ENUM, &DEFAULT_INSTANCE},

#define SHOOBY_META_MAP_END                        \
    }                                              \
    ;                                              \
    static constexpr size_t required_buffer_size() \
    {                                              \
        size_t size = 0;                           \
        for (size_t i = 0; i < NUM; i++)           \
        {                                          \
            size += META_MAP[i].size;              \
        }                                          \
        return size;                               \
    }                                              \
    }                                              \
    ;

/*

USAGE:
define a metamap for a database named "Shooby"

//1. define the metamap name
DEFINE_SHOOBY_META_MAP(Shooby)
//2. define the enums
SHOOBY_ENUMS(
    NUMBER,
    BOOL,
    STRING,
    BLOB
)

//3. define the metamap
SHOOBY_META_MAP_START
META_MAP_INTEGRAL(NUMBER, uint16_t, 250)
META_MAP_INTEGRAL(BOOL, bool, true)
META_MAP_STRING(STRING, "WHATEVER", 32)
META_MAP_BLOB(BLOB, BlobClass, some_blob_class_instance)
SHOOBY_META_MAP_END




//4. use the metamap
ShoobyDB<Shooby>::Init();
auto num = ShoobyDB<Shooby>::Get<uint16_t>(Shooby::NUMBER);
assert(num == 250);

ShoobyDB<Shooby>::Set(Shooby::NUMBER, uint16_t(500));

num = ShoobyDB<Shooby>::Get<uint16_t>(Shooby::NUMBER);
assert(num == 500);
*/

#endif