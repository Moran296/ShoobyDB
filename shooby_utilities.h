#ifndef _SHOOBY_UTILITIES_H_
#define _SHOOBY_UTILITIES_H_

#include <type_traits>
#include <concepts>
#include <variant>
#include <cstdint>

//================ UTILITY ALIASES =================

using value_variant_t = std::variant<bool, uint8_t, int8_t, uint16_t, int16_t, int32_t, uint32_t, float, const char *, const void *>;
using size_t = std::size_t;

//================ CONCEPTS =================

template <class T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <class T>
concept EnumMetaMap = requires(T) {
    typename T::enum_type;
    T::enum_type::NUM > 0;
    { T::required_buffer_size() } -> std::same_as<size_t>;
    { T::META_MAP[0].size } -> std::convertible_to<size_t>;
    { T::META_MAP[0].name } -> std::convertible_to<const char *>;
    { T::META_MAP[0].default_val} -> std::convertible_to<value_variant_t>;
};

//================ UTILITY CLASSES =================

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

#endif