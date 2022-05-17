#pragma once

#include"Application.h"

class Scene
{
public:
    virtual ~Scene()=default;
public:
    virtual void Init(){}
    virtual void ImguiUpdate(){}
    virtual void Update(TimeStep ts){}
    virtual void OnEvent(const Event* e){}
    virtual void Destory(){}
};