
template <EnumMetaMap E>
void DB<E>::Init()
{
    SHOOBY_MUTEX_INIT(s_mutex);
    Reset();

    s_is_initialized = true;
}

template <EnumMetaMap E>
void DB<E>::Init(Backend &&backend)
{
    SHOOBY_MUTEX_INIT(s_mutex);
    backend = std::move(backend);

    Reset();
    if (backend.reader != nullptr)
    {
        for (int i = 0; i < E::NUM; i++)
        {
            typename E::enum_type e = static_cast<E::enum_type>(i);
            backend.reader(get_name(e), DATA_BUFFER + get_offset(e), get_size(e), backend.user_data);
        }
    }

    s_is_initialized = true;
}

// reset buffer to default!
template <EnumMetaMap E>
void DB<E>::Reset()
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
size_t DB<E>::get_offset(E::enum_type e)
{
    int offset = 0;
    for (int i = 0; i < e; i++)
    {
        offset += E::META_MAP[i].size;
    }

    return offset;
}

template <EnumMetaMap E>
template <NotPointer T>
T DB<E>::Get(E::enum_type e)
{
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    T t;

    // case for blobs
    if constexpr (not std::is_arithmetic_v<T>)
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

    ShoobyLock lock(s_mutex);
    memcpy(&t, DATA_BUFFER + get_offset(e), get_size(e));
    return t;
}

template <EnumMetaMap E>
template <Pointer T>
T DB<E>::Get(E::enum_type e)
{
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    ShoobyLock lock(s_mutex);

    // case for strings
    if constexpr (std::is_same_v<T, const char *>)
    {
        if (not std::holds_alternative<const char *>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a string");

        return (T)(DATA_BUFFER + get_offset(e));
    }

    // case for const pointers
    else
    {
        // Do not return non const pointers to the buffer,user might use it incorrectly!
        static_assert(std::is_const_v<std::remove_pointer_t<T>>, "can't provide pointer to nonconst buffer area!");

        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a blob pointer");

        return (T)(DATA_BUFFER + get_offset(e));
    }
}

template <EnumMetaMap E>
template <E::enum_type e>
FixedString<E::META_MAP[e].size> DB<E>::GetString()
{
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    ShoobyLock lock(s_mutex);

    if (not std::holds_alternative<const char *>(E::META_MAP[e].default_val))
        ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a string");

    return FixedString<E::META_MAP[e].size>{(const char *)(DATA_BUFFER + get_offset(e))};
}

template <EnumMetaMap E>
template <class T>
bool DB<E>::Set(E::enum_type e, const T &t)
{
    using raw_type = std::decay_t<T>;
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    size_t size = get_size(e);

    if constexpr (std::is_pointer_v<raw_type>)
    {
        // case for strings
        if constexpr (std::is_same_v<char *, raw_type>)
        {
            if (not std::holds_alternative<const char *>(E::META_MAP[e].default_val))
                ON_SHOOBY_TYPE_MISMATCH("type mismatch! not a string");

            if (strlen(t) >= size)
                ON_SHOOBY_TYPE_MISMATCH("string too long!");

            size = strlen(t) + 1;
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
            ON_SHOOBY_TYPE_MISMATCH("arithmetic type mismatch!");
    }

    bool changed = false;
    {
        ShoobyLock lock(s_mutex);
        changed = set_if_changed(DATA_BUFFER + get_offset(e), t, size);
        if (changed && backend.writer != nullptr)
            backend.writer(get_name(e), DATA_BUFFER + get_offset(e), get_size(e), backend.user_data);
    }

    if (changed && observer != nullptr)
        observer(e, observer_user_data);

    return changed;
}

template <EnumMetaMap E>
template <class T>
bool DB<E>::set_if_changed(void *dst, const T &src, size_t size)
{
    using raw_type = std::decay_t<T>;
    if constexpr (std::is_pointer_v<raw_type>)
    {
        if (memcmp(dst, static_cast<const void *>(src), size) == 0)
            return false;

        memcpy(dst, src, size);
        return true;
    }
    else
    {
        if (memcmp(dst, &src, size) == 0)
            return false;

        memcpy(dst, &src, size);
        return true;
    }
}

template <EnumMetaMap E>
template <class Visitor>
void DB<E>::VisitRawEach(Visitor &visitor)
{
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    ShoobyLock lock(s_mutex);
    for (size_t i = 0; i < E::NUM; ++i)
    {
        typename E::enum_type e = static_cast<E::enum_type>(i);
        visitor(e, E::META_MAP[e], DATA_BUFFER + get_offset(e));
    }
}

template <EnumMetaMap E>
template <class Visitor>
void DB<E>::VisitRaw(E::enum_type e, Visitor &visitor)
{
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    ShoobyLock lock(s_mutex);
    visitor(e, E::META_MAP[e], DATA_BUFFER + get_offset(e));
}

template <EnumMetaMap E>
void DB<E>::SetObserver(observer_f f, void *user_data)
{
    SHOOBY_ASSERT(s_is_initialized, "DB not initialized!");
    ShoobyLock lock(s_mutex);
    observer = f;
    observer_user_data = user_data;
}