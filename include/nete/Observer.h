#pragma once

#include "Function.h"
#include "SignalBase.h"

#include <map>

namespace nete {

class Observer {
  template <typename T> friend class Signal;

protected:
  ~Observer() {
    for (const auto &p : _tracked_signals) {
      SignalBase *signal = p.first;
      DelegateKey key = p.second;
      signal->disconnect(key);
    }
  }

private:
  // managed by SignalBase
  std::vector<std::pair<SignalBase *, DelegateKey>> _tracked_signals;
};

} // namespace nete
