#include <tp/thread_label.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/string_utils.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

using wheels::Quoted;

// Fibers execution backend support: thread_local -> ThreadLocal
static twist::util::ThreadLocal<ThreadLabel> thread_label;

void LabelThread(const ThreadLabel& label) {
  *thread_label = label;
}

void ExpectThread(const ThreadLabel& label) {
  WHEELS_ASSERT(label == *thread_label,
                "Unexpected thread label: " << Quoted(*thread_label)
                                            << ", expected " << Quoted(label));
}

ThreadLabel GetThreadLabel() {
  return *thread_label;
}

}  // namespace tp
