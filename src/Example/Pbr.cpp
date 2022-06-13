#include"Pbr.h"


#include<imgui.h>

void Pbr::Init()
{
	m_model = std::make_shared<AnimationModel<>>("../assets/traveler/scene.gltf");

    const auto& aabb = m_model->GetAABB();
    auto dig = aabb.mMax - aabb.mMin;
    float lens = std::max(dig.z, std::max(dig.x, dig.y));
    TinyMath::Vec3f center(
        aabb.mMax.x + aabb.mMin.x,
        aabb.mMax.y + aabb.mMin.y,
        aabb.mMax.z + aabb.mMin.z);
    center = center / 2.0f;
    m_focus_dist = lens * 70.0f;
    m_center = Vec3f(0.0f,0.0f,0.0f);

    auto camera = Application::Get()->get_camera();
    m_shader.u_model = Scale(Mat4f::GetIdentity(), Vec3f(70.0f, 70.0f, 70.0f))* Tranlate(Mat4f::GetIdentity(), -center);
    m_shader.u_mvp = camera->get_projection_mat() * camera->get_view_mat()*m_shader.u_model;
   
    m_shader.u_light_dir = Normalize(Vec3f(0.0f, 1.0f,0.0f));

    m_shader.u_bones_matrix = m_model->get_bone_transform();
}

void Pbr::ImguiUpdate()
{
    ImGui::Begin("State");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void Pbr::Update(TimeStep ts, Input::MouseState moustate)
{
    auto camera = Application::Get()->get_camera();

    m_model->UpdateAnime(ts);

    m_shader.u_mvp = camera->get_projection_mat() * camera->get_view_mat()* m_shader.u_model;
    m_shader.u_view_pos = camera->get_position();
    uint32_t width = Application::Get()->get_width();
    uint32_t height = Application::Get()->get_height();


    Renderer::Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    Renderer::SetState(DRAW_PIXEL|FACE_CULL);
    m_model->Draw(m_shader);

    Renderer::FlushFrame();
}

void Pbr::OnEvent(const Event* e)
{
}
