#pragma once
#include"Common/Scene.h"
#include"Common/Event.h"
#include"Common/Model.h"
#include"Renderer/Math.h"
#include"Renderer/Buffer.h"
#include"Renderer/Camera.h"
#include"Renderer/FrameBuffer.h"
#include"shaders/BlinnPhongShader.h"

using namespace TinyMath;


class BlinnPhone :public Scene
{
public:
    BlinnPhone(const char* name):Scene(name){}


    void Init()override;
    void ImguiUpdate()override;
    void Update(TimeStep ts,Input::MouseState mouse_state)override;
    void OnEvent(const Event* e)override;


private:


    shared_ptr<Model<>> m_model;
    BlinnShader<> m_blinn_shader;
};