#pragma once

#include <await/executors/task.hpp>

namespace await::executors {

// Execute task immediately in the current thread ignoring exceptions
void ExecuteHere(Task& task);

}  // namespace await::executors
