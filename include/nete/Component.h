#pragma once

#include "tl/multivector.h"

#include <tuple>
#include <type_traits>
#include <vector>
#include <deque>

namespace nete {
template <typename... Args> struct Chunks {};

template <typename T, class Mapping, class EntityTraits, class ComponentTraits>
class Component
    : public Component<Chunks<T>, Mapping, EntityTraits, ComponentTraits> {};

template <typename... ChunkTypes, class Mapping, class EntityTraits,
          class ComponentTraits>
class Component<Chunks<ChunkTypes...>, Mapping, EntityTraits, ComponentTraits> {
public:
  template <unsigned ChunkIndex>
  using value_type =
      typename std::tuple_element<ChunkIndex, std::tuple<ChunkTypes...>>::type;

  using size_type = typename ComponentTraits::size_type;

  class iterator {
  public:
  private:
    size_type _index;
  };

  template <unsigned ChunkIndex> value_type<ChunkIndex> &get(iterator it);

  iterator begin();
  iterator end();

  bool empty();
  size_type size();
  void reserve(size_type new_cap);

  void push_back(const ChunkTypes &... val);

private:
  tl::multivector<tl::types<ChunkTypes...>> _storage;
};
}
