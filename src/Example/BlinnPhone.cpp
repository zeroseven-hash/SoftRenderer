#include"BlinnPhone.h"

#include"Renderer/Renderer.h"

#include<imgui.h>




void BlinnPhone::Init()
{
   
    
    Renderer::SetViewPort(Application::Get()->get_width(), Application::Get()->get_height());
    m_model = std::make_shared<Model<PosColorCoordVertex>>("../assets/nanosuit/nanosuit.obj");
    m_camera = std::make_shared<Camera>(60.0f, Application::Get()->get_width(), Application::Get()->get_height(), 0.01f, 100.0f);
    m_camera->set_distance(25.0f);
    m_camera->set_focal_point(Vec3f(0.0f, 7.0f, 0.0f));

   
    m_blinn_shader.u_mvp = m_camera->get_projection_mat() * m_camera->get_view_mat();

}

void BlinnPhone::ImguiUpdate()
{
    ImGui::Begin("State");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void BlinnPhone::Update(TimeStep ts)
{
    m_camera->update(ts.get_second(), m_mousestate);
    m_blinn_shader.u_mvp = m_camera->get_projection_mat() * m_camera->get_view_mat();
    uint32_t width = Application::Get()->get_width();
    uint32_t height = Application::Get()->get_height();
    
    
    Renderer::Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    Renderer::SetState(DRAW_PIXEL);
    m_model->Draw(m_blinn_shader);


    Renderer::FlushFrame();
}

void BlinnPhone::OnEvent(const Event* e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<MouseMoveEvent>(std::bind(&BlinnPhone::OnMouseMove, this, std::placeholders::_1));
    dispatcher.dispatch<MouseScrollEvent>(std::bind(&BlinnPhone::OnMouseScroll, this, std::placeholders::_1));
}

void BlinnPhone::OnMouseMove(const MouseMoveEvent* e)
{
    m_mousestate.x_ = e->x_;
    m_mousestate.y_ = e->y_;
    
}

void BlinnPhone::OnMouseScroll(const MouseScrollEvent* e)
{
    m_mousestate.z_ +=e->z_;
}
