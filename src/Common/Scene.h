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
private:
    std::string m_name;
};