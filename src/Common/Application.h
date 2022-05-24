#pragma once
#include <stdint.h>
#include<string>
#include<memory>

using std::shared_ptr;
class Scene;
struct Event;
struct TimeStep
{
public:
    TimeStep(float time=0.0) :m_time(time) {}

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
private:
    Application(const char* name = "App", uint32_t width = 1270, uint32_t height = 720);
public:
    virtual ~Application() { Destory(); };
    void Run();

    void Init();


    void Destory();

    void OnEvent(const Event* e);
    void set_scene(shared_ptr<Scene> scene)
    {
        m_scene = scene;
    }

    uint32_t get_width()const { return m_width; }
    uint32_t get_height()const { return m_height; }
public:
    
    static Application* CreateApp(const char* name = "App", uint32_t width = 1270, uint32_t height = 720);
    static Application* Get() { return s_instance; }
private:
    std::string m_name;
    uint32_t m_width;
    uint32_t m_height;
    float m_last_time;
    shared_ptr<Scene> m_scene;
    static Application* s_instance;
};