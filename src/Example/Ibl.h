#pragma once
#include"Common/Scene.h"

#include"Animation/AnimationModel.h"
#include"shaders/SkyBoxShader.h"
#include"shaders/IblShader.h"
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
	float m_roughness = 0.0f;
	float m_metallic = 1.0f;


	std::shared_ptr<AnimationModel<>> m_model;
	std::shared_ptr<TextureCube> m_skybox;
	std::shared_ptr<TextureCube> m_skybox_irr;
	std::shared_ptr<TextureCube> m_skybox_spec;
	std::shared_ptr<Model<>> m_sphere;
	IblShader<> m_ibl_shader;
	SkyBoxShader<> m_cube_shader;
	std::shared_ptr<Texture2D> m_brdf;
};