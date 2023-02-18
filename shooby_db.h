#ifndef _SHOOBY_DB_H_
#define _SHOOBY_DB_H_

#include "shooby_utilities.h"
#include "shooby_config.h"

// ================== META DATA CLASS =================

namespace Shooby
{
    template <class... BLOBS>
    struct MetaData
    {
        using variant_t = std::variant<bool,
                                       uint8_t,
                                       int8_t,
                                       uint16_t,
                                       int16_t,
                                       int32_t,
                                       uint32_t,
                                       float,
                                       const char *,
                                       BLOBS...>;

        template <class T>
        consteval MetaData(const char *n, T &&_default_val) : size(sizeof(T)), name(n), default_val(std::move(_default_val)) {}

        consteval MetaData(const char *n, size_t s, const char *_def_str) : size(s), name(n), default_val(_def_str) {}

        const size_t size;
        const char *name;
        const variant_t default_val;
    };

    struct Backend
    {
        typedef void (*Writer)(const char *e_name, const void *data, size_t size, void *user_data);
        typedef void (*Reader)(const char *e_name, void *data, size_t size, void *user_data);

        Writer writer{};
        Reader reader{};
        void *user_data{};
    };

    template <EnumMetaMap E>
    class DB
    {
    public:
        using meta_data_t = E::meta_data_t;
        using variant_t = E::variant_t;

        typedef void (*observer_f)(E::enum_type, void *);

        static void Init();
        static void Init(Backend &&backend);

        static void Reset();

        template <NotPointer T>
        static T Get(E::enum_type e);

        /*
        returns a const pointer to the internal buffer.
        it can still be modified in another thread, so be careful.

        Best practice for strings is to use the GetString() function.
        */
        template <Pointer T>
        static T Get(E::enum_type e);

        template <E::enum_type e>
        static FixedString<E::META_MAP[e].size> GetString();

        template <class T>
        static bool Set(E::enum_type e, const T &t);

        template <class Visitor>
        static void Visit(E::enum_type e, Visitor &visitor);

        template <class Visitor>
        static void VisitEach(Visitor &visitor);

        template <class Visitor>
        static void VisitRawEach(Visitor &visitor);

        template <class Visitor>
        static void VisitRaw(E::enum_type e, Visitor &visitor);

        static void SetObserver(observer_f f, void *user_data = nullptr);

        static const char *get_name(E::enum_type e) { return E::META_MAP[e].name; }
        static size_t get_size(E::enum_type e) { return E::META_MAP[e].size; }

    private:
        // CTORS
        DB(const DB &) = delete;
        DB &operator=(const DB &) = delete;
        DB(DB &&) = delete;
        DB &operator=(DB &&) = delete;

        // DATA RELATED
        static inline constexpr size_t required_data_buffer_size = required_buffer_size<E>();
        static inline constinit uint8_t DATA_BUFFER[required_data_buffer_size]{};
        static size_t get_offset(E::enum_type e);

        template <class T>
        static bool set_if_changed(void *dst, const T &src, size_t size);

        // INITIALIZATION RELATED
        static constinit inline bool s_is_initialized = false;

        // OBSERVER CALLBACK
        static inline observer_f observer = nullptr;
        static inline void *observer_user_data = nullptr;

        // SYNCHRONIZATION
        static inline SHOOBY_MUTEX_TYPE s_mutex{};

        // BACKEND
        static inline Backend s_backend{};
    };

#include "shooby_db_inl.hpp"

} // namespace Shooby

#endif