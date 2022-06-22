#pragma once
#include"Common/Scene.h"

#include"Renderer/TextureCube.h"
class Ibl :public Scene
{
public:
	Ibl(const char* name) :Scene(name) {}

	void Init()override;
	void ImguiUpdate()override;

	void Update(TimeStep ts, Input::MouseState mouse_state)override;
	void OnEvent(const Event* e)override;
private:
	
	std::shared_ptr<TextureCube> m_skybox;

};