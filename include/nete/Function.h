#pragma once

/**
 * This code is based on nano-signal-slot library.
 * The relevant copyright notice and the permission notice are presented below.
 *
 * Copyright (c) 2014 ApEk, NoAvailableAlias, Nano-signal-slot Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

#include <cstdint>
#include <array>

namespace nete {

using DelegateKey = std::array<std::uintptr_t, 2>;

template <typename T_rv> class Function;
template <typename T_rv, typename... Args> class Function<T_rv(Args...)> {
  template <typename T> friend class Signal;

  using Thunk = T_rv (*)(void *, Args...);

  void *m_this_ptr; // instance pointer
  Thunk m_stub_ptr; // free function pointer

  template <typename T, typename F>
  Function(T &&this_ptr, F &&stub_ptr)
      : m_this_ptr{std::forward<T>(this_ptr)},
        m_stub_ptr{std::forward<F>(stub_ptr)} {}

  /*Function (void* this_ptr, Thunk stub_ptr):
      m_this_ptr { this_ptr }, m_stub_ptr { stub_ptr } {}*/

  Function(DelegateKey const &_k)
      : m_this_ptr{reinterpret_cast<void *>(std::get<0>(_k))},
        m_stub_ptr{reinterpret_cast<Thunk>(std::get<1>(_k))} {}

public:
  template <T_rv (*fun_ptr)(Args...)> static inline Function bind() {
    return {nullptr, [](void * /*nullptr*/, Args... args) {
      return (*fun_ptr)(std::forward<Args>(args)...);
    }};
  }
  template <typename T, T_rv (T::*mem_ptr)(Args...)>
  static inline Function bind(T *pointer) {
    return {pointer, [](void *this_ptr, Args... args) {
      return (static_cast<T *>(this_ptr)->*mem_ptr)(
          std::forward<Args>(args)...);
    }};
  }
  template <typename T, T_rv (T::*mem_ptr)(Args...) const>
  static inline Function bind(T *pointer) {
    return {pointer, [](void *this_ptr, Args... args) {
      return (static_cast<T *>(this_ptr)->*mem_ptr)(
          std::forward<Args>(args)...);
    }};
  }

  inline T_rv operator()(Args... args) {
    return (*m_stub_ptr)(m_this_ptr, std::forward<Args>(args)...);
  }

  inline DelegateKey key() const {
    return {{reinterpret_cast<std::uintptr_t>(m_this_ptr),
             reinterpret_cast<std::uintptr_t>(m_stub_ptr)}};
  }
};

} // namespace nete
