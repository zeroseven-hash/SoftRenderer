#pragma once

#include"Application.h"

class Scene
{
public:
    Scene(const char* name):m_name(name){}
    virtual ~Scene()=default;
public:
    virtual void Init(){}
    virtual void ImguiUpdate(){}
    virtual void Update(TimeStep ts, Input::MouseState mouse_state){}
    virtual void OnEvent(const Event* e){}
    virtual void Destory(){}
    const std::string& get_name()const { return m_name; }
    float get_dist()const { return m_focus_dist; }
    const TinyMath::Vec3f& get_center()const { return m_center; }
protected:
    std::string m_name;
    float m_focus_dist = 1.0f;
    TinyMath::Vec3f m_center = { 0.0f,0.0f,0.0f };
};