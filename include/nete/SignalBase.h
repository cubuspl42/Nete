#pragma once

#include "Function.h"
#include "Observer.h"

#include <cassert>
#include <vector>

namespace nete {

class SignalBase {
public:
  void connect(DelegateKey key) { _slots.push_back(key); }

  void disconnect(DelegateKey key) {
    auto it = std::find(_slots.begin(), _slots.end(), key);
    assert(it != _slots.end());
    _slots.erase(it);
  }

  std::vector<DelegateKey> _slots;
};

} // namespace nete
