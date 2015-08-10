#pragma once

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace nete { namespace tl {
    // indexed from 0
    template<std::size_t I, typename... T> using nth_type_of = typename std::tuple_element<I, std::tuple<T...>>::type;
    template<typename... T> using first_type_of = nth_type_of<0, T...>;
    template<typename... T> using last_type_of = nth_type_of<sizeof...(T)-1, T...>;

    // you can't partially specialize a function in C++, this class is a workaround for that
    template<std::size_t I>
    struct index : std::integral_constant<std::size_t, I>{};
    
    struct index_negative {};
    
    template<std::size_t I>
    struct previous_index_impl
    {
        using type = index<I-1>;
    };
    
    template<>
    struct previous_index_impl<0>
    {
        using type = index_negative;
    };
    
    template<std::size_t I>
    using previous_index = typename previous_index_impl<I>::type;
    
    // a type seqence, a bit like tuple, but compile-time only
    template <typename... T>
    struct types
    {
        using tuple_type = std::tuple<T...>;
        template<std::size_t I>
        using element = nth_type_of<I, T...>;
        static constexpr std::size_t size = std::tuple_size<std::tuple<T...>>::value;
    };
    
    template<typename ForwardIt>
    void destroy(ForwardIt first, ForwardIt last) {
        using value_type = typename std::iterator_traits<ForwardIt>::value_type;
        for (; first != last; ++first) {
            first->~value_type();
        }
    }
    
    // a ceiling of integer division
    template<typename T>
    T div_ceil(T a, T b)
    {
        assert(a >= 0 && b > 0);
        return (a + b - 1) / b;
    }
    
    // next multiple of `base` that is >= n
    template<typename T>
    T next_multiple_of_gte(T n, T base)
    {
        return base * div_ceil(n, base);
    }
    
    template<std::size_t N, typename... T>
    void fill_multivector_offsets(std::array<std::size_t, N> &offsets, std::size_t size, types<T...>, index<0>)
    {
        std::size_t offset = 0;
        offsets[0] = offset;
    }
    
    template<std::size_t N, std::size_t I, typename... T>
    void fill_multivector_offsets(std::array<std::size_t, N> &offsets, std::size_t size, types<T...>, index<I> i)
    {
        using prev_value_type = nth_type_of<I-1, T...>;
        using value_type = nth_type_of<I, T...>;
        fill_multivector_offsets(offsets, size, types<T...>{}, previous_index<I>{});
        std::size_t prev_offset = offsets[I - 1];
        std::size_t offset = next_multiple_of_gte(prev_offset + sizeof(prev_value_type) * size, alignof(value_type));
        offsets[I] = offset;
    }
    
    template<typename... T>
    std::array<std::size_t, sizeof...(T)> calculate_multivector_offsets(std::size_t size)
    {
        constexpr std::size_t N = sizeof...(T);
        std::array<std::size_t, N> offsets;
        fill_multivector_offsets(offsets, size, types<T...>{}, previous_index<N>{});
        return offsets;
    }
    
    template<typename... T>
    std::size_t calculate_multivector_storage_size(const std::array<std::size_t, sizeof...(T)> &offsets,
                                                   std::size_t size)
    {
        return offsets.back() + sizeof(last_type_of<T...>) * size;
    }
    
    template<typename... T>
    void calculate_multivector_pointers_impl(std::tuple<T*...> &arrays,
                                             const std::array<std::size_t, sizeof...(T)> &offsets,
                                             void *storage, index_negative)
    {}
    
    template<std::size_t I, typename... T>
    void calculate_multivector_pointers_impl(std::tuple<T*...> &arrays,
                                             const std::array<std::size_t, sizeof...(T)> &offsets,
                                             void *storage, index<I>)
    {
        using value_type = nth_type_of<I, T...>;
        calculate_multivector_pointers_impl(arrays, offsets, storage, previous_index<I>{});
        char *storage_data = reinterpret_cast<char*>(storage);
        std::get<I>(arrays) = reinterpret_cast<value_type*>(storage_data + offsets[I]);
    }
    
    template<typename... T>
    std::tuple<T*...> calculate_multivector_pointers(const std::array<std::size_t, sizeof...(T)> &offsets,
                                                     void *storage)
    {
        constexpr std::size_t N = sizeof...(T);
        std::tuple<T*...> arrays;
        calculate_multivector_pointers_impl(arrays, offsets, storage, previous_index<N>{});
        return arrays;
    }
    
    template<typename... T>
    void multi_uninitialized_fill_impl(const std::tuple<T*...> &arrays, std::size_t first_index, std::size_t last_index, const std::tuple<T&&...> &values,
                                       index_negative)
    {}
    
    template<std::size_t I, typename... T>
    void multi_uninitialized_fill_impl(const std::tuple<T*...> &arrays, std::size_t first_index, std::size_t last_index, const std::tuple<T&&...> &values,
                                       index<I>)
    {
        constexpr std::size_t N = sizeof...(T), J = N - I - 1;
        using value_type = nth_type_of<J, T...>;
        
        value_type *first = std::get<J>(arrays) + first_index;
        value_type *last = std::get<J>(arrays) + last_index;
        std::uninitialized_fill(first, last, std::forward<value_type>(std::get<J>(values)));
        try {
            multi_uninitialized_fill_impl(arrays, first_index, last_index, values, previous_index<I>{});
        } catch(...) {
            destroy(first, last);
            throw;
        }
    }
    
    template<typename... T>
    void multi_uninitialized_fill(const std::tuple<T*...> &arrays, std::size_t first_index, std::size_t last_index, const std::tuple<T&&...> &values)
    {
        constexpr std::size_t N = sizeof...(T);
        multi_uninitialized_fill_impl(arrays, first_index, last_index, values, previous_index<N>{});
    }
    
    template<typename... T>
    void multi_uninitialized_copy_impl(const std::tuple<const T*...> &in_arrays, std::size_t size, const std::tuple<T*...> &out_arrays,
                                       index_negative)
    {}
    
    template<std::size_t I, typename... T>
    void multi_uninitialized_copy_impl(const std::tuple<const T*...> &in_arrays, std::size_t size, const std::tuple<T*...> &out_arrays,
                                       index<I>)
    {
        constexpr std::size_t N = sizeof...(T), J = N - I - 1;
        using value_type = nth_type_of<J, T...>;
        
        const value_type *first_in = std::get<J>(in_arrays);
        value_type *first_out = std::get<J>(out_arrays);
        std::uninitialized_copy(first_in, first_in+size, first_out);
        try {
            multi_uninitialized_copy_impl(in_arrays, size, out_arrays, previous_index<I>{});
        } catch(...) {
            destroy(first_out, first_out+size);
        }
    }
    
    template<typename... T>
    void multi_uninitialized_copy(const std::tuple<const T*...> &in_arrays, std::size_t size, const std::tuple<T*...> &out_arrays)
    {
        constexpr std::size_t N = sizeof...(T);
        multi_uninitialized_copy_impl(in_arrays, size, out_arrays, previous_index<N>{});
    }
    
    template<typename InputIterator, typename ForwardIterator>
    void uninitialized_move(InputIterator first, InputIterator last, ForwardIterator result)
    {
        using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
        
        for(; first != last; ++first, ++result) {
            new(static_cast<void*>(&*result)) value_type{std::move(*first)};
            first->~value_type();
        }
    }
    
    template<typename... T>
    void multi_uninitialized_move_impl(const std::tuple<const T*...> &in_arrays, std::size_t size, const std::tuple<T*...> &out_arrays,
                                       index_negative)
    {}
    
    template<std::size_t I, typename... T>
    void multi_uninitialized_move_impl(const std::tuple<const T*...> &in_arrays, std::size_t size, const std::tuple<T*...> &out_arrays,
                                       index<I>)
    {
        using value_type = nth_type_of<I, T...>;
        
        multi_uninitialized_move_impl(in_arrays, size, out_arrays, previous_index<I>{});
        const value_type *first_in = std::get<I>(in_arrays);
        value_type *first_out = std::get<I>(out_arrays);
        uninitialized_move(first_in, first_in+size, first_out);
    }
    
    template<typename... T>
    void multi_uninitialized_move(const std::tuple<const T*...> &in_arrays, std::size_t size, const std::tuple<T*...> &out_arrays)
    {
        constexpr std::size_t N = sizeof...(T);
        multi_uninitialized_move_impl(in_arrays, size, out_arrays, index<N-1>{});
    }
    
    template<typename... T>
    void multi_destroy_impl(const std::tuple<T*...> arrays, std::size_t first_index, std::size_t last_index,
                            index_negative)
    {}
    
    template<std::size_t I, typename... T>
    void multi_destroy_impl(const std::tuple<T*...> arrays, std::size_t first_index, std::size_t last_index,
                                       index<I>)
    {
        using value_type = nth_type_of<I, T...>;
        
        multi_destroy_impl(arrays, first_index, last_index, previous_index<I>{});
        value_type *first = std::get<I>(arrays) + first_index;
        value_type *last = std::get<I>(arrays) + last_index;
        destroy(first, last);
    }
    
    template<typename... T>
    void multi_destroy(const std::tuple<T*...> arrays, std::size_t first_index, std::size_t last_index)
    {
        constexpr std::size_t N = sizeof...(T);
        multi_destroy_impl(arrays, first_index, last_index, index<N-1>{});
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
    struct multivector_base;
    
    template<typename... T, class Traits>
    struct multivector_base<types<T...>, Traits>
    {
        using allocator_type = typename Traits::allocator_type;
        using storage_type = std::vector<char, allocator_type>;
        using size_type = typename Traits::size_type;
        using offset_array = std::array<std::size_t, sizeof...(T)>;
        using address_tuple = std::tuple<T*...>;
        
        multivector_base() = default;
        
        multivector_base(const allocator_type &alloc, size_type capacity, size_type size)
        : _storage(alloc), _capacity(capacity), _size(size) {
            offset_array offsets = calculate_multivector_offsets<T...>(_capacity);
            std::size_t storage_size = calculate_multivector_storage_size<T...>(offsets, _capacity);
            _storage.resize(storage_size);
            _arrays = calculate_multivector_pointers<T...>(offsets, _storage.data());
        }
        
        multivector_base(multivector_base &&x) = default;
        multivector_base &operator=(multivector_base &&x) = default;
        
        multivector_base(const multivector_base &x) = delete;
        multivector_base &operator=(const multivector_base &x) = delete;
        
        storage_type _storage;
        size_type _capacity = 0;
        size_type _size = 0;
        address_tuple _arrays;
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
    
    using multivector_base_type = multivector_base<value_types, Traits>;

    static constexpr std::size_t value_types_size = value_types::size;
    static constexpr std::size_t sizeof_value_types = sizeof_tuple_head<value_types_size, std::tuple<T...>>::value;

    static_assert(value_types_size > 0, "");

    multivector() = default;
    template<typename... Args>
    multivector(size_type n, const Args &...values);
    
    multivector(const multivector &x);
    multivector &operator=(const multivector &x);
    
    multivector(multivector &&x) = default;
    multivector &operator=(multivector &&x) = default;
    
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
    template<typename... Args>
    void resize(size_type requested_size, Args&& ...args);
    void resize(size_type requested_size);
private:
    void swap_impl(iterator first, iterator second, index_negative);
    template<std::size_t I>
    void swap_impl(iterator first, iterator second, index<I>);
    
    multivector_base_type _base;
};
    
    template<typename... T, class Traits>
    template<typename... Args>
    multivector<types<T...>, Traits>::multivector(size_type size, const Args &...values)
    {
        multi_uninitialized_fill(_base._arrays, size, std::make_tuple(values...));
    }
    
template<typename... T, class Traits>
multivector<types<T...>, Traits>::multivector(const allocator_type& a)
: _base(a)
{}
    
template<typename... T, class Traits>
multivector<types<T...>, Traits>::multivector(const multivector& x)
    : _base{x.get_allocator(), x.size(), x.size()}
{
    const std::tuple<const T*...> &x_base_arrays = x._base._arrays;
    multi_uninitialized_copy(x_base_arrays, _base._capacity, _base._arrays);
}
    
template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::operator=(const multivector& x) -> multivector &
{
    multivector tmp {x};
    std::swap(*this, tmp);
    return *this;
}
    
template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::get_allocator() const noexcept -> allocator_type
{
    return _base._storage.get_allocator();
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
    return _base._size;
}

template<typename... T, class Traits>
auto multivector<types<T...>, Traits>::capacity() const noexcept -> size_type
{
    return _base._capacity;
}

template<typename... T, class Traits>
bool multivector<types<T...>, Traits>::empty() const noexcept
{
    return _base._storage.empty();
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::reserve(size_type requested_capacity)
{
    if(requested_capacity <= capacity()) {
        return;
    }
    multivector_base_type base { _base._storage.get_allocator(), requested_capacity, size() };
    const std::tuple<const T*...> &_base_arrays = _base._arrays;
    multi_uninitialized_move(_base_arrays, _base._capacity, base._arrays);
    std::swap(_base, base);
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

template<typename... T, class Traits>
template<std::size_t I>
auto multivector<types<T...>, Traits>::data() noexcept -> pointer<I>
{
    return std::get<I>(_base._arrays);
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
    return _base._storage.data();
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
void multivector<types<T...>, Traits>::swap_impl(iterator first, iterator second, index_negative)
{}

template<typename... T, class Traits>
template<std::size_t I>
void multivector<types<T...>, Traits>::swap_impl(iterator first, iterator second, index<I>)
{
    swap_impl(first, second, previous_index<I>{});
    std::swap(get<I>(first), get<I>(second));
}

template<typename... T, class Traits>
void multivector<types<T...>, Traits>::swap(iterator first, iterator second)
{
    swap_impl(first, second, previous_index<value_types_size>{});
}

template<typename... T, class Traits>
template<typename... Args>
void multivector<types<T...>, Traits>::resize(size_type requested_size, Args&& ...args)
{
    reserve(requested_size);
    if(size() < requested_size) {
        const std::tuple<T&&...> &values = std::make_tuple(std::forward<Args>(args)...);
        multi_uninitialized_fill(_base._arrays, size(), requested_size, values);
    } else {
        multi_destroy(_base._arrays, size(), requested_size);
    }
    _base._size = requested_size;
}
    
template<typename... T, class Traits>
void multivector<types<T...>, Traits>::resize(size_type requested_size)
{
    resize(requested_size, T{}...);
}

}}
