#pragma once
#include"Animation/AnimationModel.h"
#include"Common/Scene.h"
#include"Common/Event.h"
#include"Common/Application.h"
#include"shaders/PbrShader.h"

class Pbr :public Scene
{
public:
	Pbr(const char* name):Scene(name){}
	void Init()override;
	void ImguiUpdate()override;
	void Update(TimeStep ts, Input::MouseState moustate)override;
	void OnEvent(const Event* e)override;

private:
	shared_ptr<AnimationModel<>> m_model;
	PbrShader<> m_shader;
	
};