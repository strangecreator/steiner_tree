#include <iostream>
#include <chrono>


void print_current_time(std::ostream& stream) {
    auto now = std::chrono::system_clock::now();
    auto milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    stream << milliseconds_since_epoch << '\n';
}