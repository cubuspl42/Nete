#pragma once

#include "fast_vector.h"
#include "memory.h"
#include "type_traits.h"
#include "utility.h"

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace nete {
namespace tl {

template <std::size_t N, typename... T>
void fill_multivector_offsets(std::array<std::size_t, N> &offsets,
                              std::size_t size, types<T...>, index<0>) {
  std::size_t offset = 0;
  offsets[0] = offset;
}

template <std::size_t N, int I, typename... T>
void fill_multivector_offsets(std::array<std::size_t, N> &offsets,
                              std::size_t size, types<T...>, index<I> i) {
  using prev_value_type = nth_type_of<I - 1, T...>;
  using value_type = nth_type_of<I, T...>;
  fill_multivector_offsets(offsets, size, types<T...>{}, index<I - 1>{});
  std::size_t prev_offset = offsets[I - 1];
  std::size_t offset = next_multiple_of_gte(
      prev_offset + sizeof(prev_value_type) * size, alignof(value_type));
  offsets[I] = offset;
}

template <typename... T>
std::array<std::size_t, sizeof...(T)>
calculate_multivector_offsets(std::size_t size) {
  constexpr std::size_t N = sizeof...(T);
  std::array<std::size_t, N> offsets;
  fill_multivector_offsets(offsets, size, types<T...>{}, index<N - 1>{});
  return offsets;
}

template <typename... T>
std::size_t calculate_multivector_storage_size(
    const std::array<std::size_t, sizeof...(T)> &offsets, std::size_t size) {
  return offsets.back() + sizeof(last_type_of<T...>) * size;
}

template <typename... T>
void calculate_multivector_pointers_impl(
    std::tuple<T *...> &arrays,
    const std::array<std::size_t, sizeof...(T)> &offsets, void *storage,
    index<-1>) {}

template <int I, typename... T>
void calculate_multivector_pointers_impl(
    std::tuple<T *...> &arrays,
    const std::array<std::size_t, sizeof...(T)> &offsets, void *storage,
    index<I>) {
  using value_type = nth_type_of<I, T...>;
  calculate_multivector_pointers_impl(arrays, offsets, storage, index<I - 1>{});
  char *storage_data = reinterpret_cast<char *>(storage);
  std::get<I>(arrays) =
      reinterpret_cast<value_type *>(storage_data + offsets[I]);
}

template <typename... T>
std::tuple<T *...> calculate_multivector_pointers(
    const std::array<std::size_t, sizeof...(T)> &offsets, void *storage) {
  constexpr std::size_t N = sizeof...(T);
  std::tuple<T *...> arrays;
  calculate_multivector_pointers_impl(arrays, offsets, storage, index<N - 1>{});
  return arrays;
}

struct multivector_traits {
  using allocator_type = std::allocator<char>;
  using size_type = size_t;
  static constexpr bool disable_initialization = false;
};

template <class Container>
class multivector_iterator
    : public std::iterator<std::random_access_iterator_tag,
                           typename Container::size_type> {
public:
  using base = std::iterator<std::random_access_iterator_tag,
                             typename Container::size_type>;
  using size_type = typename Container::size_type;
  using difference_type = typename base::difference_type;
  multivector_iterator(size_type rhs) : _index(rhs) {}

  size_type operator*() { return _index; }

  inline multivector_iterator &operator++() {
    ++_index;
    return *this;
  }
  inline multivector_iterator &operator--() {
    --_index;
    return *this;
  }
  inline multivector_iterator operator++(int) {
    multivector_iterator tmp(*this);
    ++_index;
    return tmp;
  }
  inline multivector_iterator operator--(int) {
    multivector_iterator tmp(*this);
    --_index;
    return tmp;
  }
  inline difference_type operator-(const multivector_iterator &rhs) {
    return _index - rhs._index;
  }
  inline multivector_iterator operator+(difference_type rhs) {
    return multivector_iterator(_index + rhs);
  }
  inline multivector_iterator operator-(difference_type rhs) {
    return multivector_iterator(_index - rhs);
  }

  friend inline multivector_iterator
  operator+(difference_type lhs, const multivector_iterator &rhs) {
    return multivector_iterator(lhs + rhs._index);
  }
  friend inline multivector_iterator
  operator-(difference_type lhs, const multivector_iterator &rhs) {
    return multivector_iterator(lhs - rhs._index);
  }

  inline multivector_iterator &operator+=(difference_type rhs) {
    _index += rhs;
    return *this;
  }
  inline multivector_iterator &operator-=(difference_type rhs) {
    _index -= rhs;
    return *this;
  }

  inline bool operator==(const multivector_iterator &rhs) {
    return _index == rhs._index;
  }
  inline bool operator!=(const multivector_iterator &rhs) {
    return _index != rhs._index;
  }
  inline bool operator>(const multivector_iterator &rhs) {
    return _index > rhs._index;
  }
  inline bool operator<(const multivector_iterator &rhs) {
    return _index < rhs._index;
  }
  inline bool operator>=(const multivector_iterator &rhs) {
    return _index >= rhs._index;
  }
  inline bool operator<=(const multivector_iterator &rhs) {
    return _index <= rhs._index;
  }

private:
  size_type _index;
};

template <typename Types, class Traits = multivector_traits>
struct multivector_base;

template <typename... T, class Traits>
struct multivector_base<types<T...>, Traits> {
  using allocator_type = typename Traits::allocator_type;
  using storage_type = fast_vector<char, allocator_type>;
  using size_type = typename Traits::size_type;
  using offset_array = std::array<std::size_t, sizeof...(T)>;
  using address_tuple = std::tuple<T *...>;

  multivector_base(const allocator_type &alloc, size_type capacity,
                   size_type size)
      : _storage(alloc), _capacity(capacity), _size(size) {
    offset_array offsets = calculate_multivector_offsets<T...>(_capacity);
    std::size_t storage_size =
        calculate_multivector_storage_size<T...>(offsets, _capacity);
    _storage.resize(storage_size);
    _arrays = calculate_multivector_pointers<T...>(offsets, _storage.data());
  }

  multivector_base(multivector_base &&x) = default;
  multivector_base &operator=(multivector_base &&x) = default;

  multivector_base(const multivector_base &x) = delete;
  multivector_base &operator=(const multivector_base &x) = delete;

  storage_type _storage;
  size_type _capacity;
  size_type _size;
  address_tuple _arrays;
};

template <typename Types, class Traits = multivector_traits> class multivector;

template <typename... T, class Traits> class multivector<types<T...>, Traits> {
public:
  template <std::size_t I> using value_type = nth_type_of<I, T...>;
  using value_types = types<T...>;
  using allocator_type = typename Traits::allocator_type;
  template <std::size_t I> using reference = value_type<I> &;
  template <std::size_t I> using const_reference = const value_type<I> &;
  using size_type = typename Traits::size_type;
  using iterator = multivector_iterator<multivector>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  template <std::size_t I> using pointer = value_type<I> *;
  template <std::size_t I> using const_pointer = const value_type<I> *;
  using initialization_t =
      typename std::conditional<Traits::disable_initialization,
                                disable_initialization_t,
                                enable_initialization_t>::type;

  using multivector_base_type = multivector_base<value_types, Traits>;

  static constexpr std::size_t value_types_size = value_types::size;
  static constexpr std::size_t sizeof_value_types =
      sizeof_tuple_head<value_types_size, std::tuple<T...>>::value;
  static constexpr initialization_t initialization_strategy =
      initialization_t{};

  static_assert(value_types_size > 0, "");
  static_assert(!Traits::disable_initialization || are_trivial<T...>::value,
                "Initialization can be disabled only for trivial types!");

  explicit multivector(const allocator_type &alloc = allocator_type{});
  template <typename... Args> multivector(size_type size, const Args &... args);
  multivector(size_type size, const allocator_type &alloc = allocator_type{});
  multivector(const multivector &x);
  multivector(multivector &&x) = default;

  multivector &operator=(const multivector &x);
  multivector &operator=(multivector &&x) = default;

  allocator_type get_allocator() const noexcept;

  template <std::size_t I> reference<I> at(size_type i);
  template <std::size_t I> const_reference<I> at(size_type i) const;
  template <std::size_t I> reference<I> get(iterator it);
  template <std::size_t I> const_reference<I> get(iterator it) const;
  template <std::size_t I> reference<I> get(reverse_iterator rit);
  template <std::size_t I> const_reference<I> get(reverse_iterator rit) const;
  template <std::size_t I> pointer<I> data() noexcept;
  template <std::size_t I> const_pointer<I> data() const noexcept;
  const byte_type *storage() const noexcept;

  iterator begin() noexcept;
  iterator end() noexcept;
  reverse_iterator rbegin() noexcept;
  reverse_iterator rend() noexcept;

  bool empty() const noexcept;
  size_type size() const noexcept;
  void reserve(size_type requested_capacity);
  size_type capacity() const noexcept;

  void clear() noexcept;
  template <typename... Args> void push_back(const Args &... args);
  void emplace_back();
  void pop_back();
  void resize(size_type requested_size);
  template <typename... Args>
  void resize(size_type requested_size, const Args &... args);
  void swap(iterator first, iterator second);

private:
  void swap_impl(iterator first, iterator second, index<-1>);
  template <int I> void swap_impl(iterator first, iterator second, index<I>);

  multivector_base_type _base;
};

template <typename... T, class Traits>
multivector<types<T...>, Traits>::multivector(const allocator_type &alloc)
    : _base(alloc, 0, 0) {}

template <typename... T, class Traits>
template <typename... Args>
multivector<types<T...>, Traits>::multivector(size_type size,
                                              const Args &... args)
    : _base{allocator_type{}, size, size} {
  std::tuple<const T &...> values{args...};
  multi_uninitialized_fill(_base._arrays, 0, size, values,
                           initialization_strategy);
}

template <typename... T, class Traits>
multivector<types<T...>, Traits>::multivector(size_type size,
                                              const allocator_type &alloc)
    : _base{alloc, size, size} {
  multi_uninitialized_construct(_base._arrays, 0, size,
                                initialization_strategy);
}

template <typename... T, class Traits>
multivector<types<T...>, Traits>::multivector(const multivector &x)
    : _base{x.get_allocator(), x.size(), x.size()} {
  const std::tuple<const T *...> &x_base_arrays = x._base._arrays;
  multi_uninitialized_copy(x_base_arrays, _base._capacity, _base._arrays,
                           initialization_strategy);
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::operator=(const multivector &x)
    -> multivector & {
  multivector tmp{x};
  std::swap(*this, tmp);
  return *this;
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::get_allocator() const noexcept
    -> allocator_type {
  return _base._storage.get_allocator();
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::at(size_type i) -> reference<I> {
  return get<I>(iterator{i});
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::at(size_type i) const
    -> const_reference<I> {
  return get<I>(iterator{i});
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::get(iterator it) -> reference<I> {
  size_type index = it - begin();
  assert(index < size());
  return *(data<I>() + index);
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::get(iterator it) const
    -> const_reference<I> {
  return const_cast<multivector *>(this)->get(it);
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::get(reverse_iterator rit)
    -> reference<I> {
  return get<I>(rit.base() - 1);
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::get(reverse_iterator rit) const
    -> const_reference<I> {
  return const_cast<multivector *>(this)->get(rit);
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::data() noexcept -> pointer<I> {
  return std::get<I>(_base._arrays);
}

template <typename... T, class Traits>
template <std::size_t I>
auto multivector<types<T...>, Traits>::data() const noexcept
    -> const_pointer<I> {
  return const_cast<multivector *>(this)->data();
}

template <typename... T, class Traits>
const byte_type *multivector<types<T...>, Traits>::storage() const noexcept {
  return _base._storage.data();
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::begin() noexcept -> iterator {
  return iterator{0};
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::end() noexcept -> iterator {
  return iterator{size()};
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::rbegin() noexcept -> reverse_iterator {
  return std::reverse_iterator<iterator>{end()};
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::rend() noexcept -> reverse_iterator {
  return std::reverse_iterator<iterator>{begin()};
}

template <typename... T, class Traits>
bool multivector<types<T...>, Traits>::empty() const noexcept {
  return _base._storage.empty();
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::size() const noexcept -> size_type {
  return _base._size;
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::reserve(size_type requested_capacity) {
  if (requested_capacity <= capacity()) {
    return;
  }
  multivector_base_type new_base{_base._storage.get_allocator(),
                                 requested_capacity, size()};
  multi_uninitialized_move(_base._arrays, _base._capacity, new_base._arrays);
  std::swap(_base, new_base);
}

template <typename... T, class Traits>
auto multivector<types<T...>, Traits>::capacity() const noexcept -> size_type {
  return _base._capacity;
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::clear() noexcept {
  resize(0);
}

template <typename... T, class Traits>
template <typename... Args>
void multivector<types<T...>, Traits>::push_back(const Args &... args) {
  resize(size() + 1, args...);
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::emplace_back() {
  resize(size() + 1);
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::pop_back() {
  resize(size() - 1);
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::resize(size_type requested_size) {
  reserve(requested_size);
  if (size() < requested_size) {
    multi_uninitialized_construct(_base._arrays, size(), requested_size,
                                  initialization_strategy);
  } else {
    multi_destroy(_base._arrays, requested_size, size(),
                  initialization_strategy);
  }
  _base._size = requested_size;
}

template <typename... T, class Traits>
template <typename... Args>
void multivector<types<T...>, Traits>::resize(size_type requested_size,
                                              const Args &... args) {
  reserve(requested_size);
  if (size() < requested_size) {
    std::tuple<const T &...> values{args...};
    multi_uninitialized_fill(_base._arrays, size(), requested_size, values,
                             initialization_strategy);
  } else {
    multi_destroy(_base._arrays, size(), requested_size,
                  initialization_strategy);
  }
  _base._size = requested_size;
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::swap(iterator first, iterator second) {
  swap_impl(first, second, index<value_types_size - 1>{});
}

template <typename... T, class Traits>
void multivector<types<T...>, Traits>::swap_impl(iterator first,
                                                 iterator second, index<-1>) {}

template <typename... T, class Traits>
template <int I>
void multivector<types<T...>, Traits>::swap_impl(iterator first,
                                                 iterator second, index<I>) {
  swap_impl(first, second, index<I - 1>{});
  std::swap(get<I>(first), get<I>(second));
}

} // namespace tl
} // namespace nete
