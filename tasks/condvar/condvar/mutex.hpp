#pragma once

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/sys.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    // Your code goes here
  }

  void Unlock() {
    // Your code goes here
    locked_.store(false);
    if (count_.load() > 0) {
      //locked_.notify_one();
    }
  }

 private:
  // ???
};

}  // namespace stdlike

