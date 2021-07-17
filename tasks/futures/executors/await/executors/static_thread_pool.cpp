#include <await/executors/static_thread_pool.hpp>

#include <await/executors/label_thread.hpp>
#include <await/executors/helpers.hpp>

namespace await::executors {

IThreadPoolPtr MakeStaticThreadPool(size_t /*threads*/,
                                    const std::string& /*name*/) {
  return nullptr;  // Not implemented
}

}  // namespace await::executors
