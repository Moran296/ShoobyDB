
template <EnumMetaMap E>
void ShoobyDB<E>::Init()
{
    SHOOBY_MUTEX_INIT(s_mutex);
    Reset();
    // Backend load

    s_is_initialized = true;
}

// reset buffer to default!
template <EnumMetaMap E>
void ShoobyDB<E>::Reset()
{
    size_t offset = 0;

    for (int i = 0; i < E::NUM; i++)
    {
        size_t size = E::META_MAP[i].size;

        std::visit(Overload{
                       [size, dst = DATA_BUFFER + offset](auto t)
                       { memcpy(dst, &t, size); },
                       [size, dst = DATA_BUFFER + offset](auto *t)
                       { memcpy(dst, t, size); },
                   },
                   E::META_MAP[i].default_val);

        offset += size;
    }
}

template <EnumMetaMap E>
size_t ShoobyDB<E>::get_offset(E::enum_type e)
{
    int offset = 0;
    for (int i = 0; i < e; i++)
    {
        offset += E::META_MAP[i].size;
    }

    return offset;
}

template <EnumMetaMap E>
template <class T>
T ShoobyDB<E>::Get(E::enum_type e)
{
    T t;

    // case for strings
    if constexpr (std::is_same_v<T, const char *>)
    {
        if (not std::holds_alternative<const char *>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a string");

        return (T)(DATA_BUFFER + get_offset(e));
    }

    // case for const pointers
    else if constexpr (std::is_pointer_v<T>)
    {
        // Do not return non const pointers to the buffer,user might use it incorrectly!
        static_assert(std::is_const_v<std::remove_pointer_t<T>>, "can't provide pointer to nonconst buffer area!");

        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a blob pointer");

        return (T)(DATA_BUFFER + get_offset(e));
    }

    // case for blobs
    else if constexpr (not std::is_arithmetic_v<T>)
    {
        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a blob");

        if (sizeof(T) != get_size(e))
            ON_SHOOBY_TYPE_MISMATCH("blob size mismatch!");
    }

    // case for arithmetics
    else
    {
        if (not std::holds_alternative<T>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not an arithmetic type");
    }

    memcpy(&t, DATA_BUFFER + get_offset(e), get_size(e));
    return t;
}

template <EnumMetaMap E>
template <class T>
bool ShoobyDB<E>::Set(E::enum_type e, const T &t)
{
    using raw_type = std::decay_t<T>;

    if constexpr (std::is_pointer_v<raw_type>)
    {
        // case for strings
        if constexpr (std::is_same_v<char *, raw_type>)
        {
            if (not std::holds_alternative<const char *>(E::META_MAP[e].default_val))
                ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a string");
        }

        // case for blob pointers
        else
        {
            if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
                ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a blob pointer");
            if (sizeof(std::remove_pointer_t<T>) != get_size(e))
                ON_SHOOBY_TYPE_MISMATCH("blob size mismatch!");
        }
    }

    // case for blobs
    else if constexpr (not std::is_arithmetic_v<T>)
    {
        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a blob");

        if (sizeof(T) != get_size(e))
            ON_SHOOBY_TYPE_MISMATCH("blob size mismatch!");
    }

    // case for arithmetics
    else
    {
        if (not std::holds_alternative<T>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not an arithmetic type");
    }

    bool changed = set_if_changed(DATA_BUFFER + get_offset(e), t, get_size(e));
    return changed;
}

template <EnumMetaMap E>
template <class T>
bool ShoobyDB<E>::set_if_changed(void *dst, const T &src, size_t size)
{
    using raw_type = std::decay_t<T>;

    if constexpr (std::is_pointer_v<raw_type>)
    {
        if (memcmp(dst, src, size) == 0)
        {
            return false;
        }

        memcpy(dst, src, size);
        return true;
    }
    else
    {
        if (memcmp(dst, &src, size) == 0)
        {
            return false;
        }

        memcpy(dst, &src, size);
        return true;
    }
}

template <EnumMetaMap E>
template <class Visitor>
void ShoobyDB<E>::VisitRaw(Visitor &&visitor)
{
    for (size_t i = 0; i < E::NUM; ++i)
    {
        typename E::enum_type e = static_cast<E::enum_type>(i);
        visitor(e, E::META_MAP[e], DATA_BUFFER + get_offset(e));
    }
}

template <EnumMetaMap E>
void ShoobyDB<E>::SetObserver(observer_f f, void *user_data)
{
    observer = f;
    observer_user_data = user_data;
}