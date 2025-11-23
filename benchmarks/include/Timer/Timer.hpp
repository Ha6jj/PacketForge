#include <chrono>
#include <iostream>
#include <string>

namespace timer {

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    Timer() : start_time_(Clock::now()) {}

    void restart() {
        start_time_ = Clock::now();
    }

    double elapsed() const {
        return std::chrono::duration<double>(Clock::now() - start_time_).count();
    }

    double elapsed_ms() const {
        return std::chrono::duration<double, std::milli>(Clock::now() - start_time_).count();
    }

private:
    TimePoint start_time_;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& message) 
        : message_(message), timer_() {}

    ~ScopedTimer() {
        const double elapsed_ms = timer_.elapsed_ms();
        std::cerr << "[" << message_ << "] took " << elapsed_ms << " ms\n";
    }

private:
    std::string message_;
    Timer timer_;
};

} // timer namespace