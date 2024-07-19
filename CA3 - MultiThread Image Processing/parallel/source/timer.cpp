#include "timer.hpp"

Timer::Timer()
{
    duration = 0;    
}

void Timer::start_timer()
{
    start_time = std::chrono::high_resolution_clock::now();
}

void Timer::stop_timer(string step)
{
    auto finnish_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>
    (finnish_time - start_time).count();
    duration = elapsed_time + duration;
    cout << step << " : " << elapsed_time << " ms" << endl;
}


void Timer::print_duration()
{
    cout << "Execution : " << duration << " ms" << endl; 
}