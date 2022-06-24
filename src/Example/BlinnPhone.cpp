#include"BlinnPhone.h"

#include"Renderer/Renderer.h"

#include<imgui.h>




void BlinnPhone::Init()
{
   
    
    Renderer::SetViewPort(Application::Get()->get_width(), Application::Get()->get_height());
    m_model = std::make_shared<Model<>>("../assets/cyborg/cyborg.obj");
    const auto& aabb = m_model->GetAABB();
    auto dig = aabb.mMax - aabb.mMin;
    float lens = std::max(dig.z, std::max(dig.x, dig.y));
    TinyMath::Vec3f center(
        aabb.mMax.x + aabb.mMin.x,
        aabb.mMax.y + aabb.mMin.y,
        aabb.mMax.z + aabb.mMin.z);
    center = center / 2.0f;
    m_focus_dist = lens * 0.4f;
    m_center = center;

    auto camera = Application::Get()->get_camera();
   
    m_blinn_shader.u_mvp = camera->get_projection_mat() * camera->get_view_mat();
    m_blinn_shader.u_model = Mat4f::GetIdentity();
    m_blinn_shader.u_light_dir = Normalize(Vec3f(0.0f, 0.0f, -1.0f));
}

void BlinnPhone::ImguiUpdate()
{
    //ImGui::ShowDemoWindow();
    ImGui::Begin("State");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void BlinnPhone::Update(TimeStep ts, Input::MouseState mouse_state)
{
    auto camera = Application::Get()->get_camera();
    
    m_blinn_shader.u_mvp = camera->get_projection_mat() * camera->get_view_mat();
    m_blinn_shader.u_view_pos = camera->get_position();
    
    
    Renderer::Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    Renderer::SetState(DRAW_PIXEL|FACE_CULL);
    m_model->Draw(m_blinn_shader);


    Renderer::FlushFrame();
}

void BlinnPhone::OnEvent(const Event* e)
{
    
}

