#pragma once
#include"Common/Scene.h"
#include"Common/Event.h"
#include"Animation/AnimationModel.h"
#include"Renderer/Camera.h"
#include"shaders/BlinnPhongShader.h"


#pragma once
#include"Renderer/Shader.h"


using namespace TinyMath;
template<typename Context = BlinnContext>
class AnimationShader :public Shader<Context>
{
public:
    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, Context& context) const
    {
        const VAO::VertexType& v = vao.get_vertex(index);
       

        Mat4f transform = Mat4f::GetZero();
        float a = 0.0f;
        for (int i = 0; i < MAX_BOUNE_INFLUENCE; i++)
        {
            if (v.bone_ids_[i] == -1) continue;
            int id = v.bone_ids_[i];
            transform = transform + (*u_bones_matrix)[id] * v.weights_[i];
            a += v.weights_[i];
        }

        
        Vec4f total_pos = transform * v.pos_;
       
        context.o_normal_ = u_model *Vec4f(v.normal_);
        context.o_normal_ = Normalize(context.o_normal_);
        context.o_coords_ = v.coords_;
        context.o_pos_ = u_model * total_pos;
        return u_mvp * total_pos;
    }

    TinyMath::Vec4f FragmentShader(const Context* context,const Context delta[2]) const
    {

        //fragment shader
        const Vec3f light_color(1.0f, 1.0f, 1.0f);
        auto& L = u_light_dir;
        auto& N = context->o_normal_;
        

        //diffuse
        Vec3f diffuse = ToLinear(m_textures[u_diffuse]->Sampler2D(context->o_coords_));
        float diff = std::max(VectorDot(N, L), 0.0f);
        diffuse = diff * diffuse * light_color;

        Vec3f emissive= ToLinear(m_textures[u_emissive]->Sampler2D(context->o_coords_));
        
        const float gamma = 1.0f / 2.2f;
        

        //tone mapping
        float exposure = 0.8f;
        Vec3f color = diffuse + emissive;
        color = Exposure(color, exposure);
        
        color = ToGammar(color);
        return TinyMath::Vec4f(color, 1.0f);
    }


public:

    //uniform 
    Mat4f u_mvp;
    Mat4f u_model;
    std::vector<Mat4f>* u_bones_matrix;
    Vec3f u_light_dir;
    Vec3f u_view_pos;
    Sampler u_diffuse = 0;
    Sampler u_specular = 1;
    Sampler u_ambient = 2;
    Sampler u_bump = 3;
    Sampler u_emissive = 4;
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
	shared_ptr<AnimationModel<>> m_model;
    AnimationShader<> m_animation_shader;

};