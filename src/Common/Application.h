#pragma once
#include <stdint.h>
#include<string>

struct TimeStep
{
public:
    TimeStep(float time) :m_time(time) {}

    float get_second() { return m_time; }
    float get_millsecond() { return m_time * 1000; }

    float operator*(float a)
    {
        return a * m_time;
    }

public:
    float m_time;
};

class Application
{
public:
    Application(const char* name = "App", uint32_t width = 1270, uint32_t height = 720);
    virtual ~Application() { Destory(); };
public:
    void Run();

    void Init();

    void ImguiUpdate();
    void Update(TimeStep ts);

    void Destory();
private:
    std::string m_name;
    uint32_t m_width;
    uint32_t m_height;
    float m_last_time;
};