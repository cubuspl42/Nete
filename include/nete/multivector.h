#pragma once

#include <tuple>
#include <type_traits>
#include <vector>

namespace nete { namespace tl {

using byte_type = char;

template<std::size_t I>
struct index {};

template <typename... T>
struct types {};

struct multivector_traits
{
    using allocator_type = std::allocator<char>;
    using size_type = size_t;
};

template<typename size_type, typename... T>
class multivector_iterator
{
public:
    multivector_iterator(size_type index) : _index(index) {}
    multivector_iterator &operator++() {
        ++_index;
        return *this;
    }
    multivector_iterator &operator--() {
        --_index;
        return *this;
    }
    size_type index() {
        return _index;
    }
private:
    size_type _index;
};

template <typename Types, class Traits = multivector_traits>
class multivector;

template<typename... T, class Traits>
class multivector<types<T...>, Traits>
{
public:
    template<std::size_t I> using value_type = typename std::tuple_element<I, std::tuple<T...>>::type;
    using tuple_type = std::tuple<T...>;
    using allocator_type = typename Traits::allocator_type;
    using storage_type = std::vector<char, allocator_type>;
    template<std::size_t I> using reference = value_type<I>&;
    template<std::size_t I> using const_reference = const value_type<I>&;
    using size_type = typename Traits::size_type;
    using iterator = multivector_iterator<size_type, T...>;
    template<std::size_t I> using pointer = value_type<I>*;
    template<std::size_t I> using const_pointer = const value_type<I>*;
    using reverse_iterator = std::reverse_iterator<iterator>;

    static constexpr std::size_t tuple_size = std::tuple_size<tuple_type>::value;

    multivector() = default;
    explicit multivector(const allocator_type& a);
    multivector(const multivector& x) = default;
    multivector(multivector&& x) = default;
    ~multivector() = default;
    multivector& operator=(const multivector& x) = default;
    multivector& operator=(multivector&& x) = default;

    allocator_type get_allocator() const noexcept;

    iterator               begin()         noexcept;
    iterator               end()           noexcept;
    reverse_iterator       rbegin()        noexcept;
    reverse_iterator       rend()          noexcept;

    size_type size() const noexcept;
    size_type capacity() const noexcept;
    bool empty() const noexcept;
    void reserve(size_type requested_capacity);

    template<std::size_t I>
    reference<I>       at(size_type i);
    template<std::size_t I>
    const_reference<I> at(size_type i) const;
    template<std::size_t I>
    reference<I> get(iterator i);
    template<std::size_t I>
    const_reference<I> get(iterator i) const;

    template<std::size_t I>
    pointer<I> data() noexcept;
    template<std::size_t I>
    const_pointer<I> data() const noexcept;
    const byte_type *storage() const noexcept;

    void emplace_back();
    void pop_back();

//    iterator insert(const_iterator position, const tuple_type& x);
//    iterator insert(const_iterator position, tuple_type&& x);
//    iterator insert(const_iterator position, size_type n, const tuple_type& x);

//    iterator erase(const_iterator position);
//    iterator erase(const_iterator first, const_iterator last);

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

template<std::size_t N, typename Tuple>
struct sizeof_tuple_head;

template<std::size_t N, typename... Args>
struct sizeof_tuple_head<N, std::tuple<Args...>> : std::integral_constant<std::size_t,
    sizeof (typename std::tuple_element<N-1, std::tuple<Args...>>::type) +
        sizeof_tuple_head<N-1, std::tuple<Args...>>::value
> {};

template<typename... Args>
struct sizeof_tuple_head<0, std::tuple<Args...>> : std::integral_constant<std::size_t, 0> {};

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
auto multivector<types<T...>, Traits>::get(iterator i) -> reference<I>
{
    assert(i.index() < size());
    return *(data<I>() + i.index());
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::get(iterator i) const -> const_reference<I>
{
    return const_cast<multivector*>(this)->get(i);
}

template<std::size_t I, class tuple_type>
auto array_address(byte_type *storage_data, std::size_t capacity)
    -> typename std::tuple_element<I, tuple_type>::type *
{
    using value_type = typename std::tuple_element<I, tuple_type>::type;
    constexpr auto offset = sizeof_tuple_head<I, tuple_type>::value;
    return reinterpret_cast<value_type*>(storage_data + offset * capacity);
}

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::data() noexcept -> pointer<I>
{
    return array_address<I, tuple_type>(_storage.data(), capacity());
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
    swap_recursive(first, second, index<tuple_size-1>{});
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
    
template<class type_tuple>
void shift_arrays_right(byte_type *storage_data, std::size_t old_capacity, std::size_t new_capacity, std::size_t size,
                        index<0>) {}

template<class type_tuple, std::size_t I>
void shift_arrays_right(byte_type *storage_data, std::size_t old_capacity, std::size_t new_capacity, std::size_t size,
                        index<I>)
{
    shift_array_right(array_address<I, type_tuple>(storage_data, old_capacity),
                      array_address<I, type_tuple>(storage_data, new_capacity), size);
    shift_arrays_right<type_tuple>(storage_data, old_capacity, new_capacity, size, index<I-1>{});
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::reallocate(size_type requested_capacity)
{
    constexpr auto sizeof_tuple = sizeof_tuple_head<tuple_size, tuple_type>::value;
    _storage.resize(sizeof_tuple * requested_capacity);
    shift_arrays_right<tuple_type>(_storage.data(), _capacity, requested_capacity, _size, index<tuple_size-1>{});
    _capacity = requested_capacity;
}
    
}}
