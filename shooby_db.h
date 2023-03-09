#ifndef _SHOOBY_DB_H_
#define _SHOOBY_DB_H_

#include "shooby_utilities.h"
#include "shooby_config.h"

// ================== META DATA CLASS =================

namespace Shooby
{

    struct MetaData
    {
        template <Arithmetic T>
        consteval MetaData(const char *n, T num_default) : size(sizeof(T)), name(n), default_val(num_default) {}

        consteval MetaData(const char *n, size_t s, const char *def_str) : size(s), name(n), default_val(def_str) {}

        template <class T>
        consteval MetaData(const char *n, const T *def_blob, size_t s = sizeof(T)) : size(s), name(n), default_val((const void *)def_blob)
        {
            SHOOBY_ASSERT(sizeof(T) == size, "blob size mismatch");
        }

        const size_t size;
        const char *name;
        const value_variant_t default_val;
    };

    // ==================== BACKEND INTERFACE ====================

    struct IBackend
    {
    public:
        virtual ~IBackend() = default;

        // called once at the beginning of the program. not mandatory
        virtual void Init(){};

        // Save changed values to the backend
        virtual void Save(const char *e_name, const void *data, size_t size) = 0;

        // Load values from the backend.
        // Should return false if the value is not found, true otherwise
        virtual bool Load(const char *e_name, void *data, size_t size) = 0;
    };

    // ================== DATABASE CLASS =================

    template <EnumMetaMap E>
    class DB
    {
    public:
        static void Init(IBackend *backend = nullptr);

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

        // Observer interface. Called when a value is changed
        struct IObserver
        {
        public:
            virtual ~IObserver() = default;
            virtual void OnChange(E::enum_type e) = 0;
        };

        static void SetObserver(IObserver *observer);

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

        // BACKEND
        static inline IBackend *s_backend{};

        // OBSERVER CALLBACK
        static inline IObserver *s_observer{};

        // SYNCHRONIZATION
        static inline SHOOBY_MUTEX_TYPE s_mutex{};
    };

#include "shooby_db_inl.hpp"

} // namespace Shooby

#endif