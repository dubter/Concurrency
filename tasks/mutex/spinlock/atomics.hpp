#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////

// Atomics

using Int64 = std::int64_t;
using AtomicInt64 = volatile Int64;

////////////////////////////////////////////////////////////////////////////////

// Atomic operations

// Atomically loads and returns the current value of the atomic variable
extern "C" Int64 AtomicLoad(AtomicInt64* address);

// Atomically stores 'value' to memory location 'address'
extern "C" void AtomicStore(AtomicInt64* address, Int64 value);

// Atomically replaces content of memory location `address` with `value`,
// returns content of the location before the call
extern "C" int AtomicExchange(AtomicInt64* address, Int64 value);
