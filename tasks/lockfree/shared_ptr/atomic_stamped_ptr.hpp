#pragma once

#include <twist/stdlike/atomic.hpp>

#include <wheels/support/assert.hpp>

#include <cstdlib>

//////////////////////////////////////////////////////////////////////

// Raw pointer = T*
// Stamp - unsigned integer
// Stamped pointer = Raw Pointer + Stamp
// Packed stamped pointer = uintptr_t

//////////////////////////////////////////////////////////////////////

// StampedPtr = Raw pointer + Stamp

template <typename T>
struct StampedPtr {
  T* raw_ptr;
  size_t stamp;

  T* operator->() const {
    return raw_ptr;
  }

  T& operator*() const {
    return *raw_ptr;
  }

  explicit operator bool() const {
    return raw_ptr != nullptr;
  }

  StampedPtr IncrementStamp() const {
    return StampedPtr{raw_ptr, stamp + 1};
  }

  StampedPtr DecrementStamp() const {
    WHEELS_VERIFY(stamp > 0, "Stamp == 0");
    return StampedPtr{raw_ptr, stamp - 1};
  }
};

//////////////////////////////////////////////////////////////////////

namespace detail {

template <typename T>
struct Packer {
  using PackedPtr = uintptr_t;

  static PackedPtr Pack(StampedPtr<T> stamped_ptr) {
    return ResetHigh16Bits((uintptr_t)stamped_ptr.raw_ptr) |
           (stamped_ptr.stamp << 48);
  }

  static StampedPtr<T> Unpack(PackedPtr packed_ptr) {
    return {GetRawPointer(packed_ptr), GetStamp(packed_ptr)};
  }

 private:
  static uintptr_t ResetHigh16Bits(uintptr_t mask) {
    return (mask << 16) >> 16;
  }

  // https://en.wikipedia.org/wiki/X86-64#Canonical_form_addresses
  static int GetBit(uintptr_t mask, size_t index) {
    return (mask >> index) & 1;
  }

  static uintptr_t ToCanonicalForm(uintptr_t ptr) {
    if (GetBit(ptr, 47) != 0) {
      return ptr | ((uintptr_t)0xFFFF << 48);
    } else {
      return ResetHigh16Bits(ptr);
    }
  }

  static size_t GetStamp(PackedPtr packed_ptr) {
    return packed_ptr >> 48;
  }

  static T* GetRawPointer(PackedPtr packed_ptr) {
    return (T*)ToCanonicalForm(packed_ptr);
  }
};

}  // namespace detail

//////////////////////////////////////////////////////////////////////

// 48-bit pointer + 16-bit stamp packed in 64-bit word

// Usage:
// asp.Store({raw_ptr, 42});
// auto ptr = asp.Load();
// if (asp) { ... }
// asp->Foo();
// asp.CompareExchangeWeak(expected_stamped_ptr, {raw_ptr, 42});

template <typename T>
class AtomicStampedPtr {
  using PackedPtr = typename detail::Packer<T>::PackedPtr;
  static_assert(sizeof(PackedPtr) == 8, "Not supported");

 public:
  static const size_t kMaxStamp = 1 << 16;

 public:
  explicit AtomicStampedPtr(T* raw_ptr) : AtomicStampedPtr({raw_ptr, 0}) {
  }

  explicit AtomicStampedPtr(StampedPtr<T> stamped_ptr)
      : packed_ptr_(Pack(stamped_ptr)) {
  }


  void Store(StampedPtr<T> stamped_ptr) {
    packed_ptr_.store(Pack(stamped_ptr));
  }

  StampedPtr<T> Load() const {
    return Unpack(packed_ptr_.load());
  }

  StampedPtr<T> Exchange(StampedPtr<T> target) {
    PackedPtr old = packed_ptr_.exchange(Pack(target));
    return Unpack(old);
  }

  bool CompareExchangeWeak(StampedPtr<T>& expected, StampedPtr<T> desired) {
    PackedPtr expected_packed = Pack(expected);
    bool succeeded =
        packed_ptr_.compare_exchange_weak(expected_packed, Pack(desired));
    if (!succeeded) {
      expected = Unpack(expected_packed);
    }
    return succeeded;
  }

 private:
  static PackedPtr Pack(StampedPtr<T> stamped_ptr) {
    return detail::Packer<T>::Pack(stamped_ptr);
  }

  static StampedPtr<T> Unpack(PackedPtr packed_ptr) {
    return detail::Packer<T>::Unpack(packed_ptr);
  }

 private:
  twist::stdlike::atomic<PackedPtr> packed_ptr_;
};
