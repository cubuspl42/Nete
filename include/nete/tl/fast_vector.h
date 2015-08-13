#pragma once

#include <memory>
#include <utility>

namespace nete {
namespace tl {
template <typename T>
class fast_vector_iterator
    : public std::iterator<std::random_access_iterator_tag, T> {
public:
  using base = std::iterator<std::random_access_iterator_tag, T>;
  using difference_type = typename base::difference_type;
  fast_vector_iterator(T *ptr) : _ptr(ptr) {}

  T &operator*() { return *_ptr; }

  T *operator->() { return _ptr; }

  inline fast_vector_iterator &operator++() {
    ++_ptr;
    return *this;
  }
  inline fast_vector_iterator &operator--() {
    --_ptr;
    return *this;
  }
  inline fast_vector_iterator operator++(int) {
    fast_vector_iterator tmp(*this);
    ++_ptr;
    return tmp;
  }
  inline fast_vector_iterator operator--(int) {
    fast_vector_iterator tmp(*this);
    --_ptr;
    return tmp;
  }
  inline difference_type operator-(const fast_vector_iterator &rhs) {
    return _ptr - rhs._ptr;
  }
  inline fast_vector_iterator operator+(difference_type rhs) {
    return fast_vector_iterator(_ptr + rhs);
  }
  inline fast_vector_iterator operator-(difference_type rhs) {
    return fast_vector_iterator(_ptr - rhs);
  }

  friend inline fast_vector_iterator
  operator+(difference_type lhs, const fast_vector_iterator &rhs) {
    return fast_vector_iterator(lhs + rhs._ptr);
  }
  friend inline fast_vector_iterator
  operator-(difference_type lhs, const fast_vector_iterator &rhs) {
    return fast_vector_iterator(lhs - rhs._ptr);
  }

  inline fast_vector_iterator &operator+=(difference_type rhs) {
    _ptr += rhs;
    return *this;
  }
  inline fast_vector_iterator &operator-=(difference_type rhs) {
    _ptr -= rhs;
    return *this;
  }

  inline bool operator==(const fast_vector_iterator &rhs) {
    return _ptr == rhs._ptr;
  }
  inline bool operator!=(const fast_vector_iterator &rhs) {
    return _ptr != rhs._ptr;
  }
  inline bool operator>(const fast_vector_iterator &rhs) {
    return _ptr > rhs._ptr;
  }
  inline bool operator<(const fast_vector_iterator &rhs) {
    return _ptr < rhs._ptr;
  }
  inline bool operator>=(const fast_vector_iterator &rhs) {
    return _ptr >= rhs._ptr;
  }
  inline bool operator<=(const fast_vector_iterator &rhs) {
    return _ptr <= rhs._ptr;
  }

private:
  T *_ptr;
};

template <typename T, class Allocator> struct fast_vector_base {
  using allocator_type = Allocator;
  using size_type = std::size_t;

  fast_vector_base(const allocator_type &alloc, size_type size)
      : _allocator(alloc), _data(size ? _allocator.allocate(size) : nullptr),
        _size(size) {}

  ~fast_vector_base() {
    if (_size) {
      _allocator.deallocate(_data, _size);
    }
  }

  fast_vector_base(fast_vector_base &&x)
      : _allocator(std::move(x._allocator)), _data(x._data), _size(x._size) {
    x._data = nullptr;
    x._size = 0;
  };

  fast_vector_base &operator=(fast_vector_base &&x) {
    std::swap(_allocator, x._allocator);
    std::swap(_data, x._data);
    std::swap(_size, x._size);
    return *this;
  };

  fast_vector_base(const fast_vector_base &x) = delete;
  fast_vector_base &operator=(const fast_vector_base &x) = delete;

  allocator_type _allocator;
  T *_data;
  size_type _size;
};

template <typename T, class Allocator = std::allocator<T>> class fast_vector {
public:
  using value_type = T;
  using allocator_type = Allocator;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = std::size_t;
  using iterator = fast_vector_iterator<T>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using pointer = value_type *;
  using const_pointer = const value_type *;

  using fast_vector_base_type = fast_vector_base<value_type, allocator_type>;

  static_assert(std::is_trivial<T>::value,
                "fast_vector supports only trivial types!");

  explicit fast_vector(const allocator_type &alloc = allocator_type{});
  fast_vector(size_type size, const value_type &val,
              const allocator_type &alloc = allocator_type{});
  fast_vector(size_type size, const allocator_type &alloc = allocator_type{});
  fast_vector(const fast_vector &x);
  fast_vector(fast_vector &&x) = default;

  fast_vector &operator=(const fast_vector &x);
  fast_vector &operator=(fast_vector &&x);

  allocator_type get_allocator() const noexcept;

  reference at(size_type i);
  const_reference at(size_type i) const;
  pointer data() noexcept;
  const_pointer data() const noexcept;

  iterator begin() noexcept;
  iterator end() noexcept;
  reverse_iterator rbegin() noexcept;
  reverse_iterator rend() noexcept;

  bool empty() const noexcept;
  size_type size() const noexcept;

  void clear() noexcept;
  void push_back(const value_type &val);
  void emplace_back();
  void pop_back();
  void resize(size_type requested_size);
  void resize(size_type requested_size, const value_type &val);

private:
  void reserve(size_type requested_size);

  fast_vector_base_type _base;
};

template <typename T, class Allocator>
fast_vector<T, Allocator>::fast_vector(const allocator_type &alloc)
    : _base(alloc, 0) {}

template <typename T, class Allocator>
fast_vector<T, Allocator>::fast_vector(size_type size, const value_type &val,
                                       const allocator_type &alloc)
    : _base{alloc, size} {
  std::uninitialized_fill(_base._data, _base._data + this->size(), val);
}

template <typename T, class Allocator>
fast_vector<T, Allocator>::fast_vector(size_type size,
                                       const allocator_type &alloc)
    : _base{alloc, size} {}

template <typename T, class Allocator>
fast_vector<T, Allocator>::fast_vector(const fast_vector &x)
    : _base{x.get_allocator(), x.size()} {
  std::uninitialized_copy(x.begin(), x.end(), begin());
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::operator=(const fast_vector &x)
    -> fast_vector & {
  fast_vector tmp{x};
  std::swap(*this, tmp);
  return *this;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::operator=(fast_vector &&x) -> fast_vector & {
  clear();
  std::swap(_base, x._base);
  return *this;
};

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::get_allocator() const noexcept
    -> allocator_type {
  return _base._allocator;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::at(size_type i) -> reference {
  return _base._data[i];
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::at(size_type i) const -> const_reference {
  return _base._data[i];
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::data() noexcept -> pointer {
  return _base._data;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::data() const noexcept -> const_pointer {
  return _base._data;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::begin() noexcept -> iterator {
  return iterator{data()};
  ;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::end() noexcept -> iterator {
  return iterator{data() + size()};
  ;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::rbegin() noexcept -> reverse_iterator {
  return std::reverse_iterator<iterator>{end()};
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::rend() noexcept -> reverse_iterator {
  return std::reverse_iterator<iterator>{begin()};
}

template <typename T, class Allocator>
bool fast_vector<T, Allocator>::empty() const noexcept {
  return size() == 0;
}

template <typename T, class Allocator>
auto fast_vector<T, Allocator>::size() const noexcept -> size_type {
  return _base._size;
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::clear() noexcept {
  resize(0);
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::push_back(const value_type &val) {
  resize(size() + 1, val);
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::emplace_back() {
  resize(size() + 1);
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::pop_back() {
  resize(size() - 1);
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::resize(size_type requested_size) {
  reserve(requested_size);
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::resize(size_type requested_size,
                                       const value_type &val) {
  reserve(requested_size);
  if (size() < requested_size) {
    std::uninitialized_fill(begin() + size(), begin() + requested_size, val);
  }
  _base._size = requested_size;
}

template <typename T, class Allocator>
void fast_vector<T, Allocator>::reserve(size_type requested_size) {
  fast_vector_base_type new_base{get_allocator(), requested_size};
  auto min_size = std::min(size(), requested_size);
  std::uninitialized_copy(begin(), begin() + min_size, new_base._data);
  std::swap(_base, new_base);
}

} // namespace tl
} // namespace nete
