#include"BlinnPhone.h"


#include<imgui.h>
#include<glad/glad.h>



void BlinnPhone::Init()
{
   
    m_renderer.Init(Application::Get()->get_width(), Application::Get()->get_height());
    m_renderer.set_render_state(true, false);
    glViewport(0, 0, Application::Get()->get_width(), Application::Get()->get_height());
    m_renderer.set_clear_color(Color(0x00, 0x00, 0x00, 0xff));


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
    
    m_renderer.Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);



    m_model->Draw(m_renderer,m_blinn_shader);
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_renderer.get_canvas());
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
