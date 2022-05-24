#pragma once
#include"Common/Scene.h"
#include"Common/Event.h"
#include"Common/Model.h"
#include"Renderer/Math.h"
#include"Renderer/Buffer.h"
#include"Renderer/Shader.h"
#include"Renderer/Camera.h"
#include"Renderer/FrameBuffer.h"

#include"rttr/registration.h"
using namespace TinyMath;
struct PosColorCoordVertex
{
    Vec4f pos_;
    Vec4f color_;
    Vec2f coords_;
};


template<typename Context= ShaderContextDefault>
class BlinnShader :public Shader<Context>
{
public:
    
    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, void* context) const
    {
        auto output = (Context*)context;
        const auto& v = vao.get_vertex(index);
        output->o_color_ = v.color_;
        output->o_coords = v.coords_;
        return u_mvp * v.pos_;
    }

    TinyMath::Vec4f FragmentShader(void* context) const
    {
        auto input = (Context*)context;
        //return input.varying_vec4f_[VARYING_COLOR];
        return TinyMath::TransformToVec4(m_textures[u_diffuse]->Sampler2D(input->o_coords));
        
    }

    
public:
    
    //uniform 
    TinyMath::Mat4f u_mvp;
    Sampler u_diffuse;
    
};


RTTR_REGISTRATION
{
    rttr::registration::class_<BlinnShader<>>("BlinnShader")
    .constructor<>()
    .property("u_diffuse",&BlinnShader<>::u_diffuse);
}

class BlinnPhone :public Scene
{
public:
    BlinnPhone() = default;


    void Init()override;
    void ImguiUpdate()override;
    void Update(TimeStep ts)override;
    void OnEvent(const Event* e)override;


    void OnMouseMove(const MouseMoveEvent* e);
    void OnMouseScroll(const MouseScrollEvent* e);
private:
    Input::MouseState m_mousestate;


    shared_ptr<Camera> m_camera;
    shared_ptr<Model<PosColorCoordVertex>> m_model;


    BlinnShader<> m_blinn_shader;


    
};