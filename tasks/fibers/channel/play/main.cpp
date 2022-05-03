#include <exe/executors/thread_pool.hpp>

#include <exe/fibers/core/api.hpp>
#include <exe/fibers/sync/channel.hpp>

#include <iostream>
#include <mutex>

using namespace exe;

int main() {
  executors::ThreadPool scheduler{/*threads=*/4};

  fibers::Go(scheduler, []() {
    fibers::Channel<int> msgs{16};

    // Producer
    fibers::Go([msgs]() mutable {
      for (int i = 0; i < 128; ++i) {
        msgs.Send(i);
      }

      // Poison pill
      msgs.Send(-1);
    });

    // Consumer
    while (true) {
      int value = msgs.Receive();
      if (value == -1) {
        break;
      }
      std::cout << "Received value = " << value << std::endl;
    }
  });

  scheduler.WaitIdle();
  scheduler.Stop();

  return 0;
}
