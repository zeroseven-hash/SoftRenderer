#pragma once
#include"Common/Scene.h"
#include"Common/Event.h"
#include"Common/AnimationModel.h"
#include"Renderer/Camera.h"
#include"shaders/BlinnPhongShader.h"


#pragma once
#include"Renderer/Shader.h"



struct AnimationContext
{
    Vec2f o_coords_;
    Vec3f o_pos_;
    Vec3f o_normal_;
    Vec3f o_tangent_;
    Vec3f o_bitangent_;

};


using namespace TinyMath;
template<typename Context = AnimationContext>
class AnimationShader :public Shader<Context>
{
public:
    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, Context& context) const
    {
        const VAO::VertexType& v = vao.get_vertex(index);
        context.o_coords_ = v.coords_;
        context.o_pos_ = u_model * v.pos_;
        context.o_normal_ = u_model * Vec4f(v.normal_);
        context.o_normal_ = Normalize(context.o_normal_);

        context.o_tangent_ = u_model * Vec4f(v.tangent_);
        context.o_tangent_ = Normalize(context.o_tangent_);

        context.o_bitangent_ = u_model * Vec4f(v.bitangent_);
        context.o_bitangent_ = Normalize(context.o_bitangent_);

      
        return u_mvp * v.pos_;
    }

    TinyMath::Vec4f FragmentShader(const Context& context) const
    {

        //fragment shader
        const Vec3f light_color(1.0f, 1.0f, 1.0f);
        auto& L = -u_light_dir;
        auto& N = context.o_normal_;
        


        //ambient;
        //Vec3f ambient = m_textures[u_ambient]->Sampler2D(context.o_coords_);
        //Vec3f ambient = light_color * 0.6f;

        //diffuse
        Vec3f diffuse = m_textures[u_diffuse]->Sampler2D(context.o_coords_);
        Vec3f ambient = light_color * diffuse*0.5f;
        float diff = std::max(VectorDot(N, L), 0.0f);
        diffuse = diff * diffuse * light_color;

        
        //const float gamma = 1.0f / 2.2f;
        return TinyMath::Vec4f(diffuse+ambient, 1.0f);
    }


public:

    //uniform 
    Mat4f u_mvp;
    Mat4f u_model;
    Vec3f u_light_dir;
    Vec3f u_view_pos;
    Sampler u_diffuse = 0;
    Sampler u_specular = 1;
    Sampler u_ambient = 2;
    Sampler u_bump = 3;
};



class SkeletalAnimation:public Scene
{
public:
	SkeletalAnimation(const char* name):Scene(name){}

	void Init()override;
	void ImguiUpdate()override;
	
	void Update(TimeStep ts, Input::MouseState mouse_state)override;
	void OnEvent(const Event* e)override;
private:
	shared_ptr<Camera> m_camera;
	shared_ptr<AnimationModel<>> m_model;
    AnimationShader<> m_animation_shader;

};