#pragma once
#include"Common/Event.h"

#include"Renderer/Camera.h"
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

    void ImguiUpdate();
    void Destory();

    void OnEvent(const Event* e);
    void AddScene(shared_ptr<Scene> scene)
    {
        m_scenes.push_back(scene);
    }

    uint32_t get_width()const { return m_width; }
    uint32_t get_height()const { return m_height; }
    shared_ptr<Camera> get_camera() { return m_camera; }


    void OnMouseMove(const MouseMoveEvent* e);
    void OnMouseScroll(const MouseScrollEvent* e);
public:
    
    static Application* CreateApp(const char* name = "App", uint32_t width = 1270, uint32_t height = 720);
    static Application* Get() { return s_instance; }
private:
    std::string m_name;
    Input::MouseState m_mousestate;
    uint32_t m_width;
    uint32_t m_height;
    float m_last_time;
    shared_ptr<Camera> m_camera;
    shared_ptr<Scene> m_selected_scene;
    std::vector<shared_ptr<Scene>> m_scenes;
    static Application* s_instance;
};