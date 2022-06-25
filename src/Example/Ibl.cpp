#include "Ibl.h"

#include"Renderer/Renderer.h"

#include"Ibl.h"
#include"Renderer/Utils.h"

#include<imgui.h>

void Ibl::Init()
{
	//m_model = std::make_shared<AnimationModel<>>("../assets/steampunk_flintlock/scene.gltf");
	m_model = std::make_shared<AnimationModel<>>("../assets/helmat/DamagedHelmet.gltf");


	m_skybox = TextureCube::Create("../assets/skybox/skyEnvHDR.dds");
	m_skybox_irr = TextureCube::Create("../assets/skybox/skyDiffuseHDR.dds");
	m_skybox_spec = TextureCube::Create("../assets/skybox/skySpecularHDR.dds");
	m_brdf = Texture2D::Create("../assets/ibl_brdf_lut.png");

	

	const auto& aabb = m_model->GetAABB();
	auto dig = aabb.mMax - aabb.mMin;
	float lens = std::max(dig.z, std::max(dig.x, dig.y));
	TinyMath::Vec3f center(
		aabb.mMax.x + aabb.mMin.x,
		aabb.mMax.y + aabb.mMin.y,
		aabb.mMax.z + aabb.mMin.z);
	center = center / 2.0f;
	m_focus_dist = lens * 30.0f;
	m_center = Vec3f(0.0f, 0.0f, 0.0f);


	m_ibl_shader.u_model = TinyMath::Rotate(TinyMath::Mat4f::GetIdentity(),90.0f,TinyMath::Vec3f(1.0f,0.0f,0.0f));
	m_ibl_shader.u_skybox_irr = m_skybox_irr;
	m_ibl_shader.u_skybox_spec = m_skybox_spec;
	m_ibl_shader.SetTexture(8, m_brdf.get());
	//m_sphere = std::make_shared<Model<>>("../assets/sphere.obj");
}

void Ibl::ImguiUpdate()
{
	ImGui::Begin("State");
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	//ImGui::SliderFloat("roughtness", &m_roughness, 0.001f, 1.0f);
	//ImGui::SliderFloat("metallic", &m_metallic, 0.001f, 1.0f);
	ImGui::End();
}

void Ibl::Update(TimeStep ts, Input::MouseState mouse_state)
{
    auto camera = Application::Get()->get_camera();


    uint32_t width = Application::Get()->get_width();
    uint32_t height = Application::Get()->get_height();


    Renderer::Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    Renderer::SetState(DRAW_PIXEL);

	//sphere render
	m_ibl_shader.u_mvp = camera->get_projection_mat() * camera->get_view_mat()*m_ibl_shader.u_model;
	m_ibl_shader.u_view_pos = camera->get_position();
	/*m_ibl_shader.roughness = m_roughness;
	m_ibl_shader.metallic = m_metallic;*/
	m_model->Draw(m_ibl_shader);
	//m_sphere->Draw(m_ibl_shader);


	//skybox render
    m_cube_shader.u_mvp = camera->get_projection_mat() * camera->get_env_view_mat();
	m_cube_shader.u_skybox = m_skybox;
	Utils::RenderCube(m_cube_shader);
    Renderer::FlushFrame();
}

void Ibl::OnEvent(const Event* e)
{
}
