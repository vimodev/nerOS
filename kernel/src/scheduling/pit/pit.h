#pragma once

#include <stdint.h>
#include "../../io/io.h"

// programmable interval timer
namespace PIT {

    // Time since boot (not accurate)
    extern double time_since_boot;

    // PIT chip's oscillations per second
    const uint64_t base_frequency = 1193182;

    void sleepd(double seconds);
    void sleep(uint64_t milliseconds);

    void set_divisor(uint16_t divisor);
    uint64_t get_frequency();
    void set_frequency(uint64_t frequency);
    void tick();

}