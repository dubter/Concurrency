#include <twist/test/with/wheels/stress.hpp>

#include <twist/ed/stdlike/thread.hpp>

#include <../../mutex.hpp>
#include <../../condvar.hpp>

#include <mutex>

class OneShotEvent {
 public:
  void Wait() {
    std::unique_lock locker(mutex_);
    while (!fired_) {
      fired_cond_.Wait(locker);
    }
  }

  void Fire() {
    std::lock_guard guard(mutex_);
    fired_ = true;
    fired_cond_.NotifyOne();
  }

 private:
  bool fired_;
  stdlike::Mutex mutex_;
  stdlike::CondVar fired_cond_;
};

void StorageTest() {
  // For AddressSanitizer
  auto event = std::make_unique<OneShotEvent>();

  twist::ed::stdlike::thread fire([&event] {
    event->Fire();
  });

  event->Wait();
  event.reset();

  fire.join();
}

TEST_SUITE(Combo) {
  TWIST_TEST_REPEAT(Event, 5s) {
    StorageTest();
  }
}
