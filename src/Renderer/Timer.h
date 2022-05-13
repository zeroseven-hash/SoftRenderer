#pragma once

#include<chrono>
#include<iostream>
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