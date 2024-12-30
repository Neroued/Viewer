#pragma once

#include <chrono>
#include <iostream>

class Timer
{
public:
    // 构造函数，开始计时
    Timer() : start_time_point(std::chrono::high_resolution_clock::now()), running(true) {}

    // 开始计时
    void start()
    {
        start_time_point = std::chrono::high_resolution_clock::now();
        running = true;
    }

    // 停止计时
    void stop()
    {
        end_time_point = std::chrono::high_resolution_clock::now();
        running = false;
    }

    void stop(const char str[])
    {
        end_time_point = std::chrono::high_resolution_clock::now();
        running = false;
        std::cout << str << ": " << elapsedMilliseconds() << "ms" << std::endl;
    }

    // 获取运行时间（秒）
    double elapsedSeconds() const
    {
        return std::chrono::duration<double>(getEndTime() - start_time_point).count();
    }

    // 获取运行时间（毫秒）
    double elapsedMilliseconds() const
    {
        return std::chrono::duration<double, std::milli>(getEndTime() - start_time_point).count();
    }

    // 获取运行时间（微秒）
    double elapsedMicroseconds() const
    {
        return std::chrono::duration<double, std::micro>(getEndTime() - start_time_point).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time_point;
    bool running;

    // 返回结束时间点（如果计时器正在运行，则使用当前时间）
    std::chrono::time_point<std::chrono::high_resolution_clock> getEndTime() const
    {
        return running ? std::chrono::high_resolution_clock::now() : end_time_point;
    }
};
