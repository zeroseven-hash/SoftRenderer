#pragma once

#include<chrono>
#include<iostream>

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x) std::cout << #x ": " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - bench_##x).count()*1000 << "ms" << std::endl;
class Timer
{
public:
    Timer() = default;
    void Start()
    {
        start = std::chrono::steady_clock::now();
    }
    void End()
    {
        end = std::chrono::steady_clock::now();
        duration = end - start;
        std::cout << duration.count() * 1000 << "ms" << std::endl;
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    std::chrono::duration <float> duration;
};