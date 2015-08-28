#pragma once

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
