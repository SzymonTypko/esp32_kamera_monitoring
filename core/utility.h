#pragma once

#include <Arduino.h>
#include <functional>

void setInterval(uint32_t intervalMs, std::function<void()> callback);
void runIntervals();