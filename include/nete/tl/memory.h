#pragma once

#include "utility.h"

#include <array>
#include <cassert>
#include <cstring>
#include <tuple>
#include <type_traits>
#include <vector>

namespace nete {
namespace tl {

struct enable_initialization_t {};
struct disable_initialization_t {};

template <typename ForwardIt> void destroy(ForwardIt first, ForwardIt last) {
  using value_type = typename std::iterator_traits<ForwardIt>::value_type;
  for (; first != last; ++first) {
    first->~value_type();
  }
}

template <int I, typename... T> struct multi_destroy_impl {
  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index,
                  enable_initialization_t initialization) {

    using value_type = nth_type_of<I, T...>;

    multi_destroy_impl<I - 1, T...>{}(arrays, first_index, last_index,
                                      initialization);
    value_type *first = std::get<I>(arrays) + first_index;
    value_type *last = std::get<I>(arrays) + last_index;
    destroy(first, last);
  }

  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, disable_initialization_t) {}
};

template <typename... T> struct multi_destroy_impl<-1, T...> {
  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, enable_initialization_t) {}
};

template <typename... T, typename initialization_t>
void multi_destroy(std::tuple<T *...> arrays, std::size_t first_index,
                   std::size_t last_index, initialization_t initialization) {
  constexpr std::size_t N = sizeof...(T);
  multi_destroy_impl<N - 1, T...>{}(arrays, first_index, last_index,
                                    initialization);
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

template <int I, typename... T> struct multi_uninitialized_construct_impl {
  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index,
                  enable_initialization_t initialization) {
    constexpr std::size_t N = sizeof...(T), J = N - I - 1;
    using value_type = nth_type_of<J, T...>;

    value_type *first = std::get<J>(arrays) + first_index;
    value_type *last = std::get<J>(arrays) + last_index;
    uninitialized_construct(first, last);
    try {
      multi_uninitialized_construct_impl<I - 1, T...>{}(
          arrays, first_index, last_index, initialization);
    } catch (...) {
      destroy(first, last);
      throw;
    }
  }

  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, disable_initialization_t) {}
};

template <typename... T> struct multi_uninitialized_construct_impl<-1, T...> {
  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, enable_initialization_t) {}
};

template <typename... T, typename initialization_t>
void multi_uninitialized_construct(std::tuple<T *...> arrays,
                                   std::size_t first_index,
                                   std::size_t last_index,
                                   initialization_t initialization) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_construct_impl<N - 1, T...>{}(arrays, first_index,
                                                    last_index, initialization);
}

template <int I, typename... T> struct multi_uninitialized_fill_impl {
  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, std::tuple<const T &...> values,
                  enable_initialization_t initialization) {
    constexpr std::size_t N = sizeof...(T), J = N - I - 1;
    using value_type = nth_type_of<J, T...>;

    value_type *first = std::get<J>(arrays) + first_index;
    value_type *last = std::get<J>(arrays) + last_index;
    std::uninitialized_fill(first, last, std::get<J>(values));
    try {
      multi_uninitialized_fill_impl<I - 1, T...>{}(
          arrays, first_index, last_index, values, initialization);
    } catch (...) {
      destroy(first, last);
      throw;
    }
  }

  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, std::tuple<const T &...> values,
                  disable_initialization_t initialization) {
    constexpr std::size_t N = sizeof...(T), J = N - I - 1;
    using value_type = nth_type_of<J, T...>;

    value_type *first = std::get<J>(arrays) + first_index;
    value_type *last = std::get<J>(arrays) + last_index;
    std::fill(first, last, std::get<J>(values));
    multi_uninitialized_fill_impl<I - 1, T...>{}(
        arrays, first_index, last_index, values, initialization);
  }
};

template <typename... T> struct multi_uninitialized_fill_impl<-1, T...> {
  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, std::tuple<const T &...> values,
                  enable_initialization_t) {}

  void operator()(std::tuple<T *...> arrays, std::size_t first_index,
                  std::size_t last_index, std::tuple<const T &...> values,
                  disable_initialization_t) {}
};

template <typename... T, typename initialization_t>
void multi_uninitialized_fill(std::tuple<T *...> arrays,
                              std::size_t first_index, std::size_t last_index,
                              std::tuple<const T &...> values,
                              initialization_t initialization) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_fill_impl<N - 1, T...>{}(arrays, first_index, last_index,
                                               values, initialization);
}

template <int I, typename... T> struct multi_uninitialized_copy_impl {
  void operator()(std::tuple<const T *...> in_arrays, std::size_t size,
                  std::tuple<T *...> out_arrays,
                  enable_initialization_t initialization) {
    constexpr std::size_t N = sizeof...(T), J = N - I - 1;
    using value_type = nth_type_of<J, T...>;

    const value_type *first_in = std::get<J>(in_arrays);
    value_type *first_out = std::get<J>(out_arrays);
    std::uninitialized_copy(first_in, first_in + size, first_out);
    try {
      multi_uninitialized_copy_impl<I - 1, T...>{}(in_arrays, size, out_arrays,
                                                   initialization);
    } catch (...) {
      destroy(first_out, first_out + size);
    }
  }

  void operator()(std::tuple<const T *...> in_arrays, std::size_t size,
                  std::tuple<T *...> out_arrays,
                  disable_initialization_t) noexcept {
    constexpr std::size_t N = sizeof...(T), J = N - I - 1;
    using value_type = nth_type_of<J, T...>;

    const value_type *first_in = std::get<J>(in_arrays);
    value_type *first_out = std::get<J>(out_arrays);
    std::memcpy(first_out, first_in, sizeof(value_type) * size);
    multi_uninitialized_copy_impl<I - 1, T...>{}(in_arrays, size, out_arrays,
                                                 disable_initialization_t{});
  }
};

template <typename... T> struct multi_uninitialized_copy_impl<-1, T...> {
  void operator()(std::tuple<const T *...> in_arrays, std::size_t size,
                  std::tuple<T *...> out_arrays, enable_initialization_t) {}
  void operator()(std::tuple<const T *...> in_arrays, std::size_t size,
                  std::tuple<T *...> out_arrays,
                  disable_initialization_t) noexcept {}
};

template <typename... T>
void multi_uninitialized_copy(std::tuple<const T *...> in_arrays,
                              std::size_t size, std::tuple<T *...> out_arrays,
                              enable_initialization_t initialization) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_copy_impl<N - 1, T...>{}(in_arrays, size, out_arrays,
                                               initialization);
}

template <typename... T>
void multi_uninitialized_copy(
    std::tuple<const T *...> in_arrays, std::size_t size,
    std::tuple<T *...> out_arrays,
    disable_initialization_t initialization) noexcept {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_copy_impl<N - 1, T...>{}(in_arrays, size, out_arrays,
                                               initialization, initialization);
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

template <int I, typename... T> struct multi_uninitialized_move_impl {
  void operator()(std::tuple<T *...> in_arrays, std::size_t size,
                  std::tuple<T *...> out_arrays) {
    using value_type = nth_type_of<I, T...>;

    multi_uninitialized_move_impl<I - 1, T...>{}(in_arrays, size, out_arrays);
    value_type *first_in = std::get<I>(in_arrays);
    value_type *first_out = std::get<I>(out_arrays);
    uninitialized_move(first_in, first_in + size, first_out);
  }
};

template <typename... T> struct multi_uninitialized_move_impl<-1, T...> {
  void operator()(std::tuple<T *...> in_arrays, std::size_t size,
                  std::tuple<T *...> out_arrays) {}
};

template <typename... T>
void multi_uninitialized_move(std::tuple<T *...> in_arrays, std::size_t size,
                              std::tuple<T *...> out_arrays) {
  constexpr std::size_t N = sizeof...(T);
  multi_uninitialized_move_impl<N - 1, T...>{}(in_arrays, size, out_arrays);
}

} // namespace tl
} // namespace nete
