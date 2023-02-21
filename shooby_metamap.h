#ifndef __SHOOBY_META_MAP_H__
#define __SHOOBY_META_MAP_H__

#include "shooby_db.h"

//================HELPERS==============================================
#define SHOOBY_TO_ENUM(NAME, Y, Z) NAME,

#define SHOOBY_NOTHING(X, Y, Z) // nothing
#define SHOOBY_STATIC_ALLOCATE_BLOB(NAME, TYPE, CTOR) static const inline TYPE def_##NAME = CTOR;

#define SHOOBY_TO_META_ARITHMETIC(ENUM, TYPE, DEFAULT) \
    {#ENUM, TYPE(DEFAULT)},
#define SHOOBY_TO_META_STRING(ENUM, DEFAULT, SIZE) \
    {#ENUM, SIZE, DEFAULT},
#define SHOOBY_TO_META_BLOB(ENUM, TYPE, DEFAULT_INSTANCE) \
    {#ENUM, &def_##ENUM},
//=====================================================================

#define DEFINE_SHOOBY_META_MAP(CONFIG_LIST)                                                          \
    struct CONFIG_LIST                                                                               \
    {                                                                                                \
        static inline constexpr const char *name = #CONFIG_LIST;                                     \
        enum enum_type                                                                               \
        {                                                                                            \
            CONFIG_LIST(SHOOBY_TO_ENUM, SHOOBY_TO_ENUM, SHOOBY_TO_ENUM)                              \
                NUM                                                                                  \
        };                                                                                           \
                                                                                                     \
        /*STATIC ALLOCATE BLOBS IN STRUCT*/                                                          \
        CONFIG_LIST(SHOOBY_NOTHING, SHOOBY_NOTHING, SHOOBY_STATIC_ALLOCATE_BLOB)                     \
                                                                                                     \
        static inline constexpr Shooby::MetaData META_MAP[NUM] =                                     \
            {                                                                                        \
                CONFIG_LIST(SHOOBY_TO_META_ARITHMETIC, SHOOBY_TO_META_STRING, SHOOBY_TO_META_BLOB)}; \
                                                                                                     \
        static inline constexpr const char *get_name(enum_type t)                                    \
        {                                                                                            \
            return META_MAP[t].name;                                                                 \
        }                                                                                            \
        static inline constexpr size_t get_size(enum_type t)                                         \
        {                                                                                            \
            return META_MAP[t].size;                                                                 \
        }                                                                                            \
    };

/*
=======================================================================
*~*~*~*~*~*~*~ THIS IS HOW WE CREATE A DATABASE METAMAP *~*~*~*~*~*~*~*
=======================================================================

1. Define a macro with 3 parameters.
 Inside use the parameters to define the members of the DB.
 The name of the macro is not important.

```
#define DB_MEMBERS(CONFIG_NUM, CONFIG_STRING, CONFIG_BLOB)     \
    CONFIG_NUM(A_NUMBER_16, uint16_t, 16)                        \
    CONFIG_NUM(SOME_BOOL, bool, true)                               \
    CONFIG_STRING(SOME_STRING, "HELLO", 10)                         \
    CONFIG_NUM(NUMBER_32, uint32_t, 32)                        \
    CONFIG_BLOB(SOME_BLOB, Bl, Bl{})
```

2. Define the DB meta map class with a name (DB_NAME),
 and the macro you defined earlier (DB_MEMBERS)

```
DEFINE_SHOOBY_META_MAP(DB_NAME, DB_MEMBERS)
```

NOTE: YOU CAN CHANGE THE NAME OF THE MACRO PARAMETERS BUT:
     ***DO NOT CHANGE THE ORDER OF THE MACRO PARAMETERS***

This is it. You can use it like this:
```
                    using enum DB_NAME::enum_type;

                    Shooby::DB<DB_NAME>::Init();
                    auto number = Shooby::DB<DB_NAME>::Get<uint16_t>(A_NUMBER_16);
                    assert(number == 16);

                    Shooby::DB<DB_NAME>::Set(A_NUMBER_16, uint16_t(32));

                    number = Shooby::DB<DB_NAME>::Get<uint16_t>(A_NUMBER_16);
                    assert(number == 32);

                    std::cout << "db name: " << DB_NAME::name << std::endl;
                    std::cout << "first config name: " << DB_NAME::get_name(A_NUMBER_16) << std::endl;
```

===============ALTERNATIVE CREATION METHOD============================

You could also define the class yourself as long as it implements EnumMetaMap concept:
struct Doobey
{
    static constexpr const char *name = "Doobey"; // < must be defined >

    enum enum_type // < enum named enum_type must be defined >
    {
        NUMBER,
        BOOL,
        STRING,
        BLOB,
        NUM  // < enum count NUM must be defined >
    };

    Bloby some_blob_class_instance = Bloby{};

    // < static inline constexpr MetaData META_MAP[enum_type::NUM] must be defined >
    static inline constexpr Shooby::MetaData META_MAP[enum_type::NUM] = {       //This line is the same as SHOOBY_META_MAP_START
        [NUMBER] = {"NUMBER", uint16_t(250)},
        [BOOL] = {"BOOL", bool(true)},
        [STRING] = {"STRING", 32, "WHATEVER"},
        [BLOB] = {"BLOB", &some_blob_class_instance},
    };

    // You can also add any other members you want if it helps your implementation
        static inline constexpr const char *get_name(enum_type t)
        {
            return META_MAP[t].name;
        }

        static inline constexpr size_t get_size(enum_type t)
        {
            return META_MAP[t].size;
        }
};

*/

#endif