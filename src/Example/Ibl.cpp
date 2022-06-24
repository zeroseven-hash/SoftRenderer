#include "Ibl.h"

#include"Renderer/Renderer.h"

#include"Ibl.h"
#include"Renderer/Utils.h"

#include<imgui.h>

void Ibl::Init()
{
	m_skybox = TextureCube::Create("../assets/skybox/skyEnvHDR.dds");
	m_center = { 0.0f,0.0f,0.0f };
	m_focus_dist = 2.0f;

}

void Ibl::ImguiUpdate()
{
	ImGui::Begin("State");
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}

void Ibl::Update(TimeStep ts, Input::MouseState mouse_state)
{
    auto camera = Application::Get()->get_camera();

	Mat4f no_trans_view = camera->get_view_mat();
	no_trans_view[0][3] = no_trans_view[1][3] = no_trans_view[2][3] = 0.0f;
	
	/*float aspect = (float)Application::Get()->get_width()/ Application::Get()->get_height();
	float orthoLeft = -0.5f * aspect * 1.0f;
	float orthoRight = 0.5f * aspect * 1.0f;
	float orthoTop = 0.5f * 1.0f;
	float orthoBottom = -0.5f * 1.0f;
	Mat4f sky_proj = TinyMath::ortho(orthoLeft,orthoRight,orthoTop,orthoBottom,0.1,100.0f);*/
    m_cube_shader.u_mvp = camera->get_projection_mat() * no_trans_view;
    uint32_t width = Application::Get()->get_width();
    uint32_t height = Application::Get()->get_height();


    Renderer::Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    Renderer::SetState(DRAW_PIXEL);
	m_cube_shader.u_skybox = m_skybox;
	Utils::RenderCube(m_cube_shader);


    Renderer::FlushFrame();
}

void Ibl::OnEvent(const Event* e)
{
}
