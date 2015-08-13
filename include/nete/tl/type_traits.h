#pragma once

#include <type_traits>

namespace nete {
namespace tl {
template <typename Head, typename... Tail>
struct are_trivial
    : std::integral_constant<bool, std::is_trivial<Head>::value &&
                                       are_trivial<Tail...>::value> {};

template <typename Head> struct are_trivial<Head> : std::is_trivial<Head> {};

} // namespace tl
} // namespace nete
