#pragma once

/**
 * This code is licensed under the Apache License.
 *
 * Copyright 2015 cubuspl42
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * See LICENSE.txt file for details.
 */

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
template <int I> struct index : std::integral_constant<int, I> {};

// a type seqence, a bit like tuple, but compile-time only
template <typename... T> struct types {
  using tuple_type = std::tuple<T...>;
  template <std::size_t I> using element = nth_type_of<I, T...>;
  static constexpr std::size_t size = std::tuple_size<std::tuple<T...>>::value;
};

// a ceiling of integer division
template <typename T> T div_ceil(T a, T b) {
  assert(a >= 0 && b > 0);
  return (a + b - 1) / b;
}

template <typename T> T is_power_of_two(T x) {
  return !(x == 0) && !(x & (x - 1));
}

// next multiple of `base` that is >= n
template <typename T> T round_up(T n, T base) {
  return base * div_ceil(n, base);
}

// next multiple of `base` that is >= n, `base` is power of two
template <typename T> T round_up_pot(T n, T base) {
  assert(is_power_of_two(base));
  return (n + base - 1) & ~(base - 1);
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
