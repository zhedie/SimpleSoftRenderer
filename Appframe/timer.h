#pragma once

#include <chrono>

class Timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
    Timer() {
        reset();
    }

    void reset() {
        start = std::chrono::high_resolution_clock::now();
    }

    float elapsed() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.001f * 0.001f * 0.001f;
    }

    float elapsedMs() {
        return elapsed() * 1000.0f;
    }
};