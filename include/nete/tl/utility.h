#pragma once

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace nete {
namespace tl {

using byte_type = char;

// indexed from 0
template <std::size_t I, typename... T>
using nth_type_of = typename std::tuple_element<I, std::tuple<T...>>::type;
template <typename... T> using first_type_of = nth_type_of<0, T...>;
template <typename... T> using last_type_of = nth_type_of<sizeof...(T)-1, T...>;

// you can't partially specialize a function in C++, this class is a workaround
// for that
template <std::size_t I>
struct index : std::integral_constant<std::size_t, I> {};

struct index_negative {};

template <std::size_t I> struct previous_index_impl {
  using type = index<I - 1>;
};

template <> struct previous_index_impl<0> { using type = index_negative; };

template <std::size_t I>
using previous_index = typename previous_index_impl<I>::type;

// a type seqence, a bit like tuple, but compile-time only
template <typename... T> struct types {
  using tuple_type = std::tuple<T...>;
  template <std::size_t I> using element = nth_type_of<I, T...>;
  static constexpr std::size_t size = std::tuple_size<std::tuple<T...>>::value;
};

template <typename ForwardIt> void destroy(ForwardIt first, ForwardIt last) {
  using value_type = typename std::iterator_traits<ForwardIt>::value_type;
  for (; first != last; ++first) {
    first->~value_type();
  }
}

template <typename... T>
void multi_destroy(const std::tuple<T *...> arrays, std::size_t first_index,
                   std::size_t last_index) {
  constexpr std::size_t N = sizeof...(T);
  multi_destroy_impl(arrays, first_index, last_index, index<N - 1>{});
}

// a ceiling of integer division
template <typename T> T div_ceil(T a, T b) {
  assert(a >= 0 && b > 0);
  return (a + b - 1) / b;
}

// next multiple of `base` that is >= n
template <typename T> T next_multiple_of_gte(T n, T base) {
  return base * div_ceil(n, base);
}

template <std::size_t N, typename Tuple> struct sizeof_tuple_head;

template <std::size_t N, typename... Args>
struct sizeof_tuple_head<N, std::tuple<Args...>>
    : std::integral_constant<
          std::size_t,
          sizeof(
              typename std::tuple_element<N - 1, std::tuple<Args...>>::type) +
              sizeof_tuple_head<N - 1, std::tuple<Args...>>::value> {};

template <typename... Args>
struct sizeof_tuple_head<0, std::tuple<Args...>>
    : std::integral_constant<std::size_t, 0> {};

} // namespace tl
} // namespace nete
