#pragma once

#include <string>

namespace tp {

using ThreadLabel = std::string;

void LabelThread(const ThreadLabel& label);

void ExpectThread(const ThreadLabel& label);

ThreadLabel GetThreadLabel();

}  // namespace tp
