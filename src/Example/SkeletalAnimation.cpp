#include "SkeletalAnimation.h"

#include"Renderer/Renderer.h"

#include"imgui.h"


void SkeletalAnimation::Init()
{

    Renderer::SetViewPort(Application::Get()->get_width(), Application::Get()->get_height());
    m_model = std::make_shared<AnimationModel<>>("../assets/kgirl/scene.gltf");
    m_camera = std::make_shared<Camera>(60.0f, Application::Get()->get_width(), Application::Get()->get_height(), 1.0f, 3000.0f);
    const auto& aabb = m_model->GetAABB();
    auto dig = aabb.mMax - aabb.mMin;
    float lens = std::max(dig.z, std::max(dig.x, dig.y));
    TinyMath::Vec3f center(
        aabb.mMax.x + aabb.mMin.x,
        aabb.mMax.y + aabb.mMin.y,
        aabb.mMax.z + aabb.mMin.z);
    center = center / 2.0f;
    m_camera->set_distance(lens * 1.5f);
    m_camera->set_focal_point(center);


    m_animation_shader.u_mvp = m_camera->get_projection_mat() * m_camera->get_view_mat();
    m_animation_shader.u_model = Mat4f::GetIdentity();
    m_animation_shader.u_light_dir = Normalize(Vec3f(0.0f, 0.0f, -1.0f));
}

void SkeletalAnimation::ImguiUpdate()
{
    ImGui::Begin("State");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void SkeletalAnimation::Update(TimeStep ts, Input::MouseState mouse_state)
{
    m_camera->update(ts.get_second(), mouse_state);

    m_animation_shader.u_mvp = m_camera->get_projection_mat() * m_camera->get_view_mat();
    m_animation_shader.u_view_pos = m_camera->get_position();
    uint32_t width = Application::Get()->get_width();
    uint32_t height = Application::Get()->get_height();


    Renderer::Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    Renderer::SetState(DRAW_PIXEL);
    m_model->Draw(m_animation_shader);
    Renderer::FlushFrame();
}

void SkeletalAnimation::OnEvent(const Event* e)
{
}
