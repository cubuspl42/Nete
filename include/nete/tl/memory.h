#pragma once

#include "utility.h"

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace nete {
namespace tl {
template <typename ForwardIt> void destroy(ForwardIt first, ForwardIt last) {
  using value_type = typename std::iterator_traits<ForwardIt>::value_type;
  for (; first != last; ++first) {
    first->~value_type();
  }
}

template <typename... T>
void multi_destroy_impl(std::tuple<T *...> arrays, std::size_t first_index,
                        std::size_t last_index, index<-1>) {}

template <int I, typename... T>
void multi_destroy_impl(std::tuple<T *...> arrays, std::size_t first_index,
                        std::size_t last_index, index<I>) {
  using value_type = nth_type_of<I, T...>;

  multi_destroy_impl(arrays, first_index, last_index, index<I - 1>{});
  value_type *first = std::get<I>(arrays) + first_index;
  value_type *last = std::get<I>(arrays) + last_index;
  destroy(first, last);
}

template <typename... T>
void multi_destroy(std::tuple<T *...> arrays, std::size_t first_index,
                   std::size_t last_index) {
  constexpr std::size_t N = sizeof...(T);
  multi_destroy_impl(arrays, first_index, last_index, index<N - 1>{});
}

template <class ForwardIterator>
void uninitialized_construct(ForwardIterator first, ForwardIterator last) {
  using value_type = typename std::iterator_traits<ForwardIterator>::value_type;

  ForwardIterator current = first;
  try {
    for (; current != last; ++current) {
      ::new (static_cast<void *>(std::addressof(*current))) value_type{};
    }
  } catch (...) {
    destroy(first, current);
    throw;
  }
}

template <typename... T>
void multi_uninitialized_construct_impl(std::tuple<T *...> arrays,
                                        std::size_t first_index,
                                        std::size_t last_index, index<-1>) {}

template <int I, typename... T>
void multi_uninitialized_construct_impl(std::tuple<T *...> arrays,
                                        std::size_t first_index,
                                        std::size_t last_index, index<I>) {
  constexpr std::size_t N = sizeof...(T), J = N - I - 1;
  using value_type = nth_type_of<J, T...>;

  value_type *first = std::get<J>(arrays) + first_index;
  value_type *last = std::get<J>(arrays) + last_index;
  uninitialized_construct(first, last);
  try {
    multi_uninitialized_construct_impl(arrays, first_index, last_index,
                                       index<I - 1>{});
  } catch (...) {
    destroy(first, last);
    throw;
  }
}

template <typename... T>
void multi_uninitialized_construct(std::tuple<T *...> arrays,
                                   std::size_t first_index,
                                   std::size_t last_index) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_construct_impl(arrays, first_index, last_index,
                                     index<N - 1>{});
}

template <typename... T>
void multi_uninitialized_fill_impl(std::tuple<T *...> arrays,
                                   std::size_t first_index,
                                   std::size_t last_index,
                                   const std::tuple<const T &...> &values,
                                   index<-1>) {}

template <int I, typename... T>
void multi_uninitialized_fill_impl(std::tuple<T *...> arrays,
                                   std::size_t first_index,
                                   std::size_t last_index,
                                   const std::tuple<const T &...> &values,
                                   index<I>) {
  constexpr std::size_t N = sizeof...(T), J = N - I - 1;
  using value_type = nth_type_of<J, T...>;

  value_type *first = std::get<J>(arrays) + first_index;
  value_type *last = std::get<J>(arrays) + last_index;
  std::uninitialized_fill(
      first, last, std::forward<const value_type &>(std::get<J>(values)));
  try {
    multi_uninitialized_fill_impl(arrays, first_index, last_index, values,
                                  index<I - 1>{});
  } catch (...) {
    destroy(first, last);
    throw;
  }
}

template <typename... T>
void multi_uninitialized_fill(std::tuple<T *...> arrays,
                              std::size_t first_index, std::size_t last_index,
                              const std::tuple<const T &...> &values) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_fill_impl(arrays, first_index, last_index, values,
                                index<N - 1>{});
}

template <typename... T>
void multi_uninitialized_copy_impl(const std::tuple<const T *...> &in_arrays,
                                   std::size_t size,
                                   std::tuple<T *...> out_arrays, index<-1>) {}

template <int I, typename... T>
void multi_uninitialized_copy_impl(const std::tuple<const T *...> &in_arrays,
                                   std::size_t size,
                                   std::tuple<T *...> out_arrays, index<I>) {
  constexpr std::size_t N = sizeof...(T), J = N - I - 1;
  using value_type = nth_type_of<J, T...>;

  const value_type *first_in = std::get<J>(in_arrays);
  value_type *first_out = std::get<J>(out_arrays);
  std::uninitialized_copy(first_in, first_in + size, first_out);
  try {
    multi_uninitialized_copy_impl(in_arrays, size, out_arrays, index<I - 1>{});
  } catch (...) {
    destroy(first_out, first_out + size);
  }
}

template <typename... T>
void multi_uninitialized_copy(const std::tuple<const T *...> &in_arrays,
                              std::size_t size, std::tuple<T *...> out_arrays) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_copy_impl(in_arrays, size, out_arrays, index<N - 1>{});
}

template <typename InputIterator, typename ForwardIterator>
void uninitialized_move(InputIterator first, InputIterator last,
                        ForwardIterator result) {
  using value_type = typename std::iterator_traits<ForwardIterator>::value_type;

  for (; first != last; ++first, ++result) {
    new (static_cast<void *>(&*result)) value_type{std::move(*first)};
    first->~value_type();
  }
}

template <typename... T>
void multi_uninitialized_move_impl(std::tuple<T *...> in_arrays,
                                   std::size_t size,
                                   std::tuple<T *...> out_arrays, index<-1>) {}

template <int I, typename... T>
void multi_uninitialized_move_impl(std::tuple<T *...> in_arrays,
                                   std::size_t size,
                                   std::tuple<T *...> out_arrays, index<I>) {
  using value_type = nth_type_of<I, T...>;

  multi_uninitialized_move_impl(in_arrays, size, out_arrays, index<I - 1>{});
  value_type *first_in = std::get<I>(in_arrays);
  value_type *first_out = std::get<I>(out_arrays);
  uninitialized_move(first_in, first_in + size, first_out);
}

template <typename... T>
void multi_uninitialized_move(std::tuple<T *...> in_arrays, std::size_t size,
                              std::tuple<T *...> out_arrays) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_move_impl(in_arrays, size, out_arrays, index<N - 1>{});
}

} // namespace tl
} // namespace nete
