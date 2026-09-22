#include "utility.h"

#include <utility>
#include <vector>

namespace {
struct Interval {
  uint32_t intervalMs;
  uint32_t lastRun;
  std::function<void()> callback;
};

std::vector<Interval> intervals;
}

void setInterval(uint32_t intervalMs, std::function<void()> callback) {
  intervals.push_back({intervalMs, millis(), std::move(callback)});
}

void runIntervals() {
  const uint32_t now = millis();
  const size_t intervalCount = intervals.size();

  for (size_t index = 0; index < intervalCount; ++index) {
    Interval& interval = intervals[index];
    if (now - interval.lastRun >= interval.intervalMs) {
      interval.lastRun = now;
      interval.callback();
    }
  }
}