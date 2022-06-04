#pragma once
#include"Renderer/Shader.h"



struct BlinnContext
{
    Vec2f o_coords_;
    Vec3f o_pos_;
    Vec3f o_normal_;
    Vec3f o_tangent_;
    Vec3f o_bitangent_;
};


using namespace TinyMath;
template<typename Context = BlinnContext>
class BlinnShader :public Shader<Context>
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
        Vec3f normal = m_textures[u_bump]->Sampler2D(context.o_coords_);
        static Mat3f TBN;
        TBN.SetCol(0, context.o_tangent_);
        TBN.SetCol(1, context.o_bitangent_);
        TBN.SetCol(2, context.o_normal_);

        const Vec3f light_color(1.0f, 1.0f, 1.0f);
        auto& L = -u_light_dir;
        //auto& N = context.o_normal_;
        static Vec3f N;
        N = Normalize(TBN * normal);
        auto& V = Normalize(u_view_pos - context.o_pos_);


        //ambient;
        //Vec3f ambient = m_textures[u_ambient]->Sampler2D(context.o_coords_);
        Vec3f ambient = light_color * 0.1f;

        //diffuse
        Vec3f diffuse = m_textures[u_diffuse]->Sampler2D(context.o_coords_);
        float diff = std::max(VectorDot(N, L), 0.0f);
        diffuse = diff * diffuse * light_color;

        // specular
        Vec3f specular = m_textures[u_specular]->Sampler2D(context.o_coords_);
        //Half Vector
        auto H = Normalize(L + V);
        float spec = std::pow(std::max(VectorDot(N, H), 0.0f), 32.0f);
        specular = spec * specular * light_color;

        Vec3f color = ambient + diffuse + specular;
        //const float gamma = 1.0f / 2.2f;
        return TinyMath::Vec4f(color, 1.0f);
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
