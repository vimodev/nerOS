#include "pit.h"

namespace PIT {

    double time_since_boot = 0;

    uint16_t divisor = 65535;

    // THE FOLLOWING SLEEP FUNCTIONS ARE VIOLENTLY INACCURATE
    // but are still useful for some kernel waiting stuff

    // Halt for given amount of seconds
    void sleepd(double seconds) {
        double start_time = time_since_boot;
        while (time_since_boot < start_time + seconds) {
            asm("hlt");
        }
    }

    // Halt for given amount of milliseconds
    void sleep(uint64_t milliseconds) {
        sleepd((double) milliseconds / 1000);
    }

    // Set the PIT chip's divisor level
    void set_divisor(uint16_t divisor) {
        // Under 100 is stupid
        if (divisor < 100) divisor = 100;
        PIT::divisor = divisor;
        outb(0x40, (uint8_t) (divisor & 0x00ff));
        io_wait();
        outb(0x40, (uint8_t)((divisor & 0xff00) >> 8));
    }

    // Get the PIT chip's frequency
    uint64_t get_frequency() {
        return base_frequency / divisor;
    }

    // Set the PIT chip's frequency
    void set_frequency(uint64_t frequency) {
        set_divisor(base_frequency / frequency);
    }

    // Tick the clock
    void tick() {
        time_since_boot += 1 / (double) get_frequency();
    }

}