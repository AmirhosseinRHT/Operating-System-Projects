#ifndef TIMER_HPP
#define TIMER_HPP

#include "defs.hpp"

class Timer
{
public:
    Timer();
    void start_timer();
    void stop_timer(string step);
    void print_duration();
private:
    std::chrono::milliseconds::rep duration;
    std::chrono::high_resolution_clock::time_point start_time;
};

#endif