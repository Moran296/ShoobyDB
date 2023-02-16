
#include <iostream> //remove

// reset buffer to default!
template <EnumMetaMap E>
void ShoobyDB<E>::Reset()
{
    using namespace std;

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
            std::cout << " ** type mismatch!**not a string" << std::endl;

        return (T)(DATA_BUFFER + get_offset(e));
    }

    // case for const pointers
    else if constexpr (std::is_pointer_v<T>)
    {
        // Do not return non const pointers to the buffer,user might use it incorrectly!
        static_assert(std::is_const_v<std::remove_pointer_t<T>>, "can't provide pointer to nonconst buffer area!");

        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            std::cout << " ** type mismatch!** not a blob pointer" << std::endl;

        return (T)(DATA_BUFFER + get_offset(e));

        // case for blobs
    }
    else if constexpr (not std::is_arithmetic_v<T>)
    {
        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            std::cout << " ** type mismatch!** " << std::endl;

        if (sizeof(T) != get_size(e))
            std::cout << " ** blob size mismatch!** " << std::endl;

        // case for arithmetics
    }
    else
    {
        if (not std::holds_alternative<T>(E::META_MAP[e].default_val))
        {
            std::cout << " ** type mismatch! integral ** " << std::endl;
        }
    }

    memcpy(&t, DATA_BUFFER + get_offset(e), get_size(e));
    return t;
}

template <EnumMetaMap E>
template <class T>
void ShoobyDB<E>::Set(E::enum_type e, const T &t)
{
    using raw_type = std::decay_t<T>;

    if constexpr (std::is_pointer_v<raw_type>)
    {
        // case for strings
        if constexpr (std::is_same_v<char *, raw_type>)
        {
            if (not std::holds_alternative<const char *>(E::META_MAP[e].default_val))
                std::cout << " ** type mismatch!** not a string" << std::endl;
            // case for blob pointers
        }
        else
        {
            if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
                std::cout << " ** type mismatch!** not a blob pointer" << std::endl;
            if (sizeof(std::remove_pointer_t<T>) != get_size(e))
                std::cout << " ** blob size mismatch!** " << std::endl;
        }

        memcpy(DATA_BUFFER + get_offset(e), t, get_size(e));
        return;
    }

    // case for blobs
    if constexpr (not std::is_arithmetic_v<T>)
    {
        if (not std::holds_alternative<const void *>(E::META_MAP[e].default_val))
            std::cout << " ** type mismatch!** " << std::endl;

        if (sizeof(T) != get_size(e))
            std::cout << " ** blob size mismatch!** " << std::endl;

        // case for arithmetics
    }
    else
    {
        if (not std::holds_alternative<T>(E::META_MAP[e].default_val))
        {
            std::cout << " ** type mismatch! integral ** " << std::endl;
        }
    }

    memcpy(DATA_BUFFER + get_offset(e), &t, get_size(e));
}
