#pragma once

#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace nete { namespace tl {
    
    template<typename T>
    T* align_array(std::size_t size, void*& ptr, std::size_t& space)
    {
        std::align(alignof(T), sizeof(T)*size, ptr, space);
        T *aligned_ptr = reinterpret_cast<T*>(ptr);
        space -= sizeof(T)*size;
        ptr = static_cast<char*>(ptr) + sizeof(T)*size;
        return aligned_ptr;
    }
    
    template<typename Head, typename... Tail>
    void multi_uninitialized_fill(char *ptr, std::size_t size, std::size_t capacity, std::size_t space,
                                  const Head &value, const Tail &...values)
    {
        Head *first = align_array<Head>(capacity, ptr, space);
        std::uninitialized_fill(first, first+size, value);
        try {
            multi_uninitialized_fill(ptr, size, capacity, values...);
        } catch(...) {
            Head *last = first + size;
            for (; first != last; ++first) {
                first->~Value();
            }
            throw;
        }
    }
    
template<std::size_t N, typename Tuple>
struct sizeof_tuple_head;

template<std::size_t N, typename... Args>
struct sizeof_tuple_head<N, std::tuple<Args...>> : std::integral_constant<std::size_t,
    sizeof (typename std::tuple_element<N-1, std::tuple<Args...>>::type) +
        sizeof_tuple_head<N-1, std::tuple<Args...>>::value
> {};

template<typename... Args>
struct sizeof_tuple_head<0, std::tuple<Args...>> : std::integral_constant<std::size_t, 0> {};


using byte_type = char;
    
template<std::size_t I, typename... T> using nth_type_of = typename std::tuple_element<I, std::tuple<T...>>::type;

template<std::size_t I>
struct index {};

template <typename... T>
struct types
{
    using tuple_type = std::tuple<T...>;
    template<std::size_t I>
    using element = nth_type_of<I, T...>;
    static constexpr std::size_t size = std::tuple_size<std::tuple<T...>>::value;
};

struct multivector_traits
{
    using allocator_type = std::allocator<char>;
    using size_type = size_t;
};

template<class Container>
class multivector_iterator : public std::iterator<std::random_access_iterator_tag, int>
{
    using size_type = typename Container::size_type;
public:
    multivector_iterator(size_type rhs) : _index(rhs) {}

    inline multivector_iterator& operator+=(difference_type rhs) {_index += rhs; return *this;}
    inline multivector_iterator& operator-=(difference_type rhs) {_index -= rhs; return *this;}
    /* inline value_type& operator*(); */
    /* inline value_type* operator->(); */
    /* inline value_type& operator[](const int& rhs); */

    inline multivector_iterator& operator++() {++_index; return *this;}
    inline multivector_iterator& operator--() {--_index; return *this;}
    inline multivector_iterator operator++(int) {multivector_iterator tmp(*this); ++_index; return tmp;}
    inline multivector_iterator operator--(int) {multivector_iterator tmp(*this); --_index; return tmp;}
    inline difference_type operator-(const multivector_iterator& rhs) {return _index-rhs._index;}
    inline multivector_iterator operator+(difference_type rhs) {return multivector_iterator(_index+rhs);}
    inline multivector_iterator operator-(difference_type rhs) {return multivector_iterator(_index-rhs);}
    friend inline multivector_iterator operator+(difference_type lhs, const multivector_iterator& rhs)
        {return multivector_iterator(lhs+rhs._index);}
    friend inline multivector_iterator operator-(difference_type lhs, const multivector_iterator& rhs)
        {return multivector_iterator(lhs-rhs._index);}

    inline bool operator==(const multivector_iterator& rhs) {return _index == rhs._index;}
    inline bool operator!=(const multivector_iterator& rhs) {return _index != rhs._index;}
    inline bool operator>(const multivector_iterator& rhs) {return _index > rhs._index;}
    inline bool operator<(const multivector_iterator& rhs) {return _index < rhs._index;}
    inline bool operator>=(const multivector_iterator& rhs) {return _index >= rhs._index;}
    inline bool operator<=(const multivector_iterator& rhs) {return _index <= rhs._index;}
private:
    size_type _index;
};

template <typename Types, class Traits = multivector_traits>
class multivector;

template<typename... T, class Traits>
class multivector<types<T...>, Traits>
{
public:
    template<std::size_t I> using value_type = nth_type_of<I, T...>;
    using value_types = types<T...>;
    using allocator_type = typename Traits::allocator_type;
    using storage_type = std::vector<char, allocator_type>;
    template<std::size_t I> using reference = value_type<I>&;
    template<std::size_t I> using const_reference = const value_type<I>&;
    using size_type = typename Traits::size_type;
    using iterator = multivector_iterator<multivector>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    template<std::size_t I> using pointer = value_type<I>*;
    template<std::size_t I> using const_pointer = const value_type<I>*;

    static constexpr std::size_t value_types_size = value_types::size;
    static constexpr std::size_t sizeof_value_types = sizeof_tuple_head<value_types_size, std::tuple<T...>>::value;

    static_assert(value_types_size > 0, "");

    multivector() = default;
    template<typename Head, typename... Tail>
    multivector(size_type n, const Head &value, const Tail &...values);
    explicit multivector(const allocator_type& a);

    allocator_type get_allocator() const noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;
    reverse_iterator rbegin() noexcept;
    reverse_iterator rend() noexcept;

    size_type size() const noexcept;
    size_type capacity() const noexcept;
    bool empty() const noexcept;
    void reserve(size_type requested_capacity);

    template<std::size_t I>
    reference<I> at(size_type i);
    template<std::size_t I>
    const_reference<I> at(size_type i) const;
    template<std::size_t I>
    reference<I> get(iterator it);
    template<std::size_t I>
    const_reference<I> get(iterator it) const;
    template<std::size_t I>
    reference<I> get(reverse_iterator rit);
    template<std::size_t I>
    const_reference<I> get(reverse_iterator rit) const;

    template<std::size_t I>
    pointer<I> data() noexcept;
    template<std::size_t I>
    const_pointer<I> data() const noexcept;
    const byte_type *storage() const noexcept;

    void emplace_back();
    void pop_back();

    void swap(iterator first, iterator second);
    void clear() noexcept;
    void resize(size_type requested_size);
private:
    void reallocate(size_type requested_capacity);
    void swap_recursive(iterator first, iterator second, index<0>);
    template<std::size_t I>
    void swap_recursive(iterator first, iterator second, index<I>);

    storage_type _storage;
    size_type _size = 0;
    size_type _capacity = 0;
};
    
    template<typename... T, class Traits>
    template<typename Head, typename... Tail>
    multivector<types<T...>, Traits>::multivector(size_type n, const Head &value, const Tail &...values)
    : _storage(sizeof_value_types*n)
    {
        multi_uninitialized_fill(_storage.data(), n, n, value, values...);
    }
    
template<typename... T, class Traits>
multivector<types<T...>, Traits>::multivector(const allocator_type& a)
: _storage(a)
{}
    
template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::get_allocator() const noexcept -> allocator_type
{
    return _storage.get_allocator();
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::begin() noexcept -> iterator
{
    return iterator{0};
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::end() noexcept -> iterator
{
    return iterator{size()};
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::rbegin() noexcept -> reverse_iterator
{
    return std::reverse_iterator<iterator>{end()};
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::rend() noexcept -> reverse_iterator
{
    return std::reverse_iterator<iterator>{begin()};
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::size() const noexcept -> size_type
{
    return _size;
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::capacity() const noexcept -> size_type
{
    return _capacity;
}

template<typename... T, class Traits>
bool multivector<types<T...>, Traits>::empty() const noexcept
{
    return _storage.empty();
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::reserve(size_type requested_capacity)
{
    if(requested_capacity > capacity()) {
        reallocate(requested_capacity);
    }
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::at(size_type i) -> reference<I>
{
    return get<I>(iterator{i});
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::at(size_type i) const -> const_reference<I>
{
    return get<I>(iterator{i});
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::get(iterator it) -> reference<I>
{
    size_type index = it - begin();
    assert(index < size());
    return *(data<I>() + index);
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::get(iterator it) const -> const_reference<I>
{
    return const_cast<multivector*>(this)->get(it);
}
    
template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::get(reverse_iterator rit) -> reference<I>
{
    return get<I>(rit.base() - 1);
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::get(reverse_iterator rit) const -> const_reference<I>
{
    return const_cast<multivector*>(this)->get(rit);
}

template<class Types, std::size_t I>
auto array_address(byte_type *storage_data, std::size_t capacity)
    -> typename Types::template element<I> *
{
    constexpr auto offset = sizeof_tuple_head<I, typename Types::tuple_type>::value;
    return reinterpret_cast<typename Types::template element<I> *>(storage_data + offset * capacity);
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::data() noexcept -> pointer<I>
{
    return array_address<value_types, I>(_storage.data(), capacity());
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::data() const noexcept -> const_pointer<I>
{
    return const_cast<multivector*>(this)->data();
}

template<typename... T, class Traits>
const byte_type * multivector<types<T...>, Traits>::storage() const noexcept
{
    return _storage.data();
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::emplace_back()
{
    resize(size()+1);
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::pop_back()
{
    resize(size()-1);
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::swap_recursive(iterator first, iterator second, index<0>)
{
    std::swap(get<0>(first), get<0>(second));
}

template<typename... T, class Traits>
template<std::size_t I>
void multivector<types<T...>, Traits>::swap_recursive(iterator first, iterator second, index<I>)
{
    swap_recursive(first, second, index<I-1>{});
    std::swap(get<I>(first), get<I>(second));
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::swap(iterator first, iterator second)
{
    swap_recursive(first, second, index<value_types_size-1>{});
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::resize(size_type requested_size)
{
    if(requested_size > _capacity) {
        reallocate(requested_size);
    }
    _size = requested_size;
}

template<typename T>
void shift_array_right(const T *old_address, T *new_address, std::size_t size)
{
    std::copy(std::reverse_iterator<const T*>(old_address + size), std::reverse_iterator<const T*>(old_address),
              std::reverse_iterator<T*>(new_address + size));
}

template<class Types>
void shift_arrays_right(byte_type *storage_data, std::size_t old_capacity, std::size_t new_capacity, std::size_t size,
                        index<0>) {}

template<class Types, std::size_t I>
void shift_arrays_right(byte_type *storage_data, std::size_t old_capacity, std::size_t new_capacity, std::size_t size,
                        index<I>)
{
    shift_array_right(array_address<Types, I>(storage_data, old_capacity),
                      array_address<Types, I>(storage_data, new_capacity), size);
    shift_arrays_right<Types>(storage_data, old_capacity, new_capacity, size, index<I-1>{});
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::reallocate(size_type requested_capacity)
{
    using tuple_type = std::tuple<T...>;
    constexpr auto sizeof_tuple = sizeof_tuple_head<value_types_size, tuple_type>::value;
    _storage.resize(sizeof_tuple * requested_capacity);
    shift_arrays_right<value_types>(_storage.data(), _capacity, requested_capacity, _size, index<value_types_size-1>{});
    _capacity = requested_capacity;
}

}}
