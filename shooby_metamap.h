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

#define SHOOBY_META_MAP_END  };};

/*
================= USAGE =================

define a metamap for a database named "Shooby"

** DEFINITION **
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

You could also define the class yourself as long as it implements EnumMetaMap concept:
struct Shooby
{
    static constexpr const char *name = "shooby"; // < must be defined >

    enum enum_type // < enum named enum_type must be defined >
    {
        NUMBER,
        BOOL,
        STRING,
        BLOB,
        NUM  // < enum count NUM must be defined >
    };

    // < static inline constexpr MetaData META_MAP[enum_type::NUM] must be defined >
    static inline constexpr MetaData META_MAP[enum_type::NUM] = {       //This line is the same as SHOOBY_META_MAP_START
        [NUMBER] = {"NUMBER", uint16_t(250)},
        [BOOL] = {"BOOL", bool(true)},
        [STRING] = {"STRING", 32, "WHATEVER"},
        [BLOB] = {"BLOB", &some_blob_class_instance},
    };

    // You can also add any other members you want if it helps your implementation
};


** USAGE **
//4. use the metamap
ShoobyDB<Shooby>::Init();
auto num = ShoobyDB<Shooby>::Get<uint16_t>(Shooby::NUMBER);
assert(num == 250);

ShoobyDB<Shooby>::Set(Shooby::NUMBER, uint16_t(500));

num = ShoobyDB<Shooby>::Get<uint16_t>(Shooby::NUMBER);
assert(num == 500);
*/

#endif