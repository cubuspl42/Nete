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

#include "Function.h"
#include "Observer.h"
#include "SignalBase.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace nete {

template <typename T_rv> class Signal;
template <typename T_rv, typename... Args> class Signal<T_rv(Args...)> {
private:
  template <typename T>
  void connect_method(DelegateKey const &key,
                      typename T::Observer *observer) { // SFINAE
    _base.connect(key);
    auto &tracked_signals = observer->_tracked_signals;
    tracked_signals.emplace_back(&_base, key);
  }

  template <typename T>
  void connect_method(DelegateKey const &key, ...) { // SFINAE
    _base.connect(key);
  }

  template <typename T>
  void disconnect_method(DelegateKey const &key,
                         typename T::Observer *observer) { // SFINAE
    _base.disconnect(key);
    auto &tracked_signals = observer->_tracked_signals;
    auto it = std::find(tracked_signals.begin(), tracked_signals.end(),
                        std::make_pair(&_base, key));
    assert(it != tracked_signals.end());
    tracked_signals.erase(it);
  }

  template <typename T>
  void disconnect_method(DelegateKey const &key, ...) { // SFINAE
    _base.disconnect(key);
  }

public:
  using Delegate = Function<T_rv(Args...)>;

  //-------------------------------------------------------------------CONNECT

  template <T_rv (*fun_ptr)(Args...)> void connect() {
    _base.connect(Delegate::template bind<fun_ptr>().key());
  }

  template <typename T, T_rv (T::*mem_ptr)(Args...)> void connect(T *observer) {
    auto key = Delegate::template bind<T, mem_ptr>(observer).key();
    connect_method<T>(key, observer);
  }
  template <typename T, T_rv (T::*mem_ptr)(Args...) const>
  void connect(T *observer) {
    auto key = Delegate::template bind<T, mem_ptr>(observer).key();
    connect_method<T>(key, observer);
  }

  template <typename T, T_rv (T::*mem_ptr)(Args...)> void connect(T &observer) {
    connect<T, mem_ptr>(std::addressof(observer));
  }
  template <typename T, T_rv (T::*mem_ptr)(Args...) const>
  void connect(T &observer) {
    connect<T, mem_ptr>(std::addressof(observer));
  }

  //----------------------------------------------------------------DISCONNECT

  template <T_rv (*fun_ptr)(Args...)> void disconnect() {
    _base.disconnect(Delegate::template bind<fun_ptr>().key());
  }

  template <typename T, T_rv (T::*mem_ptr)(Args...)>
  void disconnect(T *observer) {
    auto key = Delegate::template bind<T, mem_ptr>(observer).key();
    disconnect_method<T>(key, observer);
  }
  template <typename T, T_rv (T::*mem_ptr)(Args...) const>
  void disconnect(T *observer) {
    auto key = Delegate::template bind<T, mem_ptr>(observer).key();
    disconnect_method<T>(key, observer);
  }

  template <typename T, T_rv (T::*mem_ptr)(Args...)>
  void disconnect(T &observer) {
    disconnect<T, mem_ptr>(std::addressof(observer));
  }

  template <typename T, T_rv (T::*mem_ptr)(Args...) const>
  void disconnect(T &observer) {
    disconnect<T, mem_ptr>(std::addressof(observer));
  }

  //----------------------------------------------------------------------EMIT

  void operator()(Args... args) {
    for (DelegateKey key : _base._slots) {
      Delegate{key}(std::forward<Args>(args)...);
    }
  }
  template <typename Accumulator>
  void operator()(Args... args, Accumulator sink) {
    for (DelegateKey key : _base._slots) {
      sink(Delegate{key}(std::forward<Args>(args)...));
    }
  }

private:
  SignalBase _base;
};

template <class Emitter, typename T_rv> class ObjectSignal;
template <class Emitter, typename T_rv, typename... Args>
class ObjectSignal<Emitter, T_rv(Args...)> : public Signal<T_rv(Args...)> {
  friend Emitter;

private: // only for Emmiter
  void operator()(Args... args) {
    Signal<T_rv(Args...)>::operator()(std::forward<Args>(args)...);
  }

  template <typename Accumulator>
  void operator()(Args... args, Accumulator sink) {
    Signal<T_rv(Args...)>::operator()(std::forward<Args>(args)...,
                                      std::forward<Accumulator>(sink));
  }
};

} // namespace nete
