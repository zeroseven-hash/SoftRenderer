#pragma once
#include"Common/Scene.h"
#include"Common/Event.h"
#include"Common/Model.h"
#include"Renderer/Math.h"
#include"Renderer/Buffer.h"
#include"Renderer/Shader.h"
#include"Renderer/Renderer.h"
#include"Renderer/Camera.h"


#include"rttr/registration.h"
using namespace TinyMath;
struct PosColorCoordVertex
{
    Vec4f pos_;
    Vec4f color_;
    Vec2f coords_;
};

class BlinnShader :public Shader
{
public:
    
    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, ShaderContext& output) const
    {
        const auto& v = vao.get_vertex(index);
        output.varying_vec4f_[VARYING_COLOR] = v.color_;
        output.varying_vec2f_[VARYING_COORD] = v.coords_;
        return u_mvp*v.pos_;
    }

    TinyMath::Vec4f FragmentShader(ShaderContext& input) const
    {
        Vec2f corrds = input.varying_vec2f_[VARYING_COORD];
        //return input.varying_vec4f_[VARYING_COLOR];
        return m_textures[u_diffuse]->Sample2D(corrds).TransformToVec();
    }

    
public:
    const int VARYING_COLOR = 0;
    const int VARYING_COORD = 1;
    
    //uniform 
    TinyMath::Mat4f u_mvp;
    Sampler u_diffuse;
    
};


RTTR_REGISTRATION
{
    rttr::registration::class_<BlinnShader>("BlinnShader")
    .constructor<>()
    .property("u_diffuse",&BlinnShader::u_diffuse);
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


    BlinnShader m_blinn_shader;

    Renderer m_renderer;

    
};