#pragma once

#include <cstdint>

namespace tests::support {

uint16_t FindFreePort();

// Static for current process
uint16_t StaticFreePort();

}  // namespace tests::support
