#ifndef _SHOOBY_UTILITIES_H_
#define _SHOOBY_UTILITIES_H_

#include <type_traits>
#include <concepts>
#include <variant>
#include <cstdint>
#include <cstring>
#include "shooby_config.h"

//================ UTILITY ALIASES =================

using value_variant_t = std::variant<bool, uint8_t, int8_t, uint16_t, int16_t, int32_t, uint32_t, float, const char *, const void *>;
using size_t = std::size_t;

//================ CONCEPTS =================

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <class T>
concept Pointer = std::is_pointer_v<T>;

template <class T>
concept NotPointer = not
Pointer<T>;

template <class T>
concept EnumMetaMap = requires(T) {
                          typename T::enum_type;
                          requires std::is_enum_v<typename T::enum_type>;
                          T::enum_type::NUM > 0;
                          {
                              T::name
                              } -> std::convertible_to<const char *>;
                          {
                              T::META_MAP[0].size
                              } -> std::convertible_to<size_t>;
                          {
                              T::META_MAP[0].name
                              } -> std::convertible_to<const char *>;
                          {
                              T::META_MAP[0].default_val
                              } -> std::convertible_to<value_variant_t>;
                      };

// ================== UTILITY FUNCTIONS =================

template <EnumMetaMap T>
static consteval size_t required_buffer_size()
{
    size_t size = 0;
    for (size_t i = 0; i < T::NUM; i++)
    {
        size += T::META_MAP[i].size;
    }

    return size;
}

//================ UTILITY CLASSES =================

template <size_t N>
class FixedString
{
public:
    FixedString() = default;
    FixedString(const char *str) { std::strncpy(buffer, str, N); }
    FixedString(const FixedString &other) { std::strncpy(buffer, other.buffer, N); }
    FixedString(FixedString &&other) { buffer = std::move(other.buffer); }

    FixedString &operator=(const FixedString &other)
    {
        std::strncpy(buffer, other.buffer, N);
        return *this;
    }

    FixedString &operator=(FixedString &&other)
    {
        std::strncpy(buffer, other.buffer, N);
        return *this;
    }

    const char *c_str() const { return buffer; }

private:
    char buffer[N]{};
};

template <typename... Ts>
struct Overload : Ts...
{
    using Ts::operator()...;
};

/*
    example usages:

    1.
    auto overload = Overload{
        [](int i) { std::cout << "int: " << i << std::endl; },
        [](double d) { std::cout << "double: " << d << std::endl; },
        [](auto x) { std::cout << "other: " << x << std::endl; }
    };

    2.
    std::variant<int, double, std::string> v = 12.34;
    std::visit(Overload{
        [](auto& arg) { },
        [](auto* arg) { },
        [](auto arg) { }}, v);

*/

class ShoobyLock
{
public:
    ShoobyLock(SHOOBY_MUTEX_TYPE &m) : locked(m) { SHOOBY_LOCK(locked); }
    ~ShoobyLock() { SHOOBY_UNLOCK(locked); }

    ShoobyLock(const ShoobyLock &) = delete;
    ShoobyLock &operator=(const ShoobyLock &) = delete;
    ShoobyLock(ShoobyLock &&) = delete;
    ShoobyLock &operator=(ShoobyLock &&) = delete;

private:
    SHOOBY_MUTEX_TYPE &locked;
};

#endif