#pragma once
#include"BlinnPhongShader.h"

using namespace TinyMath;
template<typename Context = BlinnContext>
class PbrShader :public Shader<Context>
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


        context.o_coords_ = v.coords_;


        context.o_tangent_ = u_model  *transform *Vec4f(v.tangent_);
        context.o_tangent_ = Normalize(context.o_tangent_);

        context.o_bitangent_ = u_model  * transform* Vec4f(v.bitangent_);
        context.o_bitangent_ = Normalize(context.o_bitangent_);

        context.o_normal_ = u_model * transform * Vec4f(v.normal_);
        context.o_normal_ = Normalize(context.o_normal_);

        context.o_coords_ = v.coords_;
        context.o_pos_ = u_model *total_pos;
        return u_mvp * total_pos;
    }

    TinyMath::Vec4f FragmentShader(const Context* context) const
    {
        //set data;
        const float pi = 3.14158926f;
       


        Vec3f normal = m_textures[u_normal]->Sampler2D(context[0].o_coords_);
        normal = normal * 2.0f - Vec3f(1.0f, 1.0f, 1.0f);
        Vec3f albedo = ToLinear(m_textures[u_albedo]->Sampler2D(context[0].o_coords_));
        Vec3f metallic_roughness = m_textures[u_metalness_roughness]->Sampler2D(context[0].o_coords_);
        float metallic = metallic_roughness.b_;
        float roughness = metallic_roughness.g_;
        static Mat3f TBN;
        TBN.SetCol(0, context[0].o_tangent_);
        TBN.SetCol(1, context[0].o_bitangent_);
        TBN.SetCol(2, context[0].o_normal_);

        const Vec3f light_color(0.7f, 0.7f, 1.0f);

        static Vec3f N;
        N = Normalize(TBN * normal);
        auto& V = Normalize(u_view_pos - context[0].o_pos_);
        
        Vec3f f0 = { 0.04,0.04,0.04 };
        f0 = Mix(f0, albedo, metallic);

        float NdotV = Between(0.0f, 1.0f, VectorDot(N, V));
        Vec3f direct_color{ 0.0,0.0f,0.0f };
        for (int i = 0; i < 1; i++)
        {
            //diffuse
            const float dirlight_strength = 3.0f;
            auto L = Normalize(context[0].o_pos_- u_light_dir);
            auto H = Normalize(L + V);

            
            Vec3f radiance = light_color* dirlight_strength;
            Vec3f diffuse = (1.0f-metallic)*albedo / pi;
            
            float HdotL = Between(0.0f, 1.0f, VectorDot(H, L));
            float NdotH = Between(0.0f, 1.0f, VectorDot(N, H));
            float NdotL = Between(0.0f, 1.0f, VectorDot(N, L));
            Vec3f specular = F(HdotL, f0) * NDF(NdotH, roughness) * Vis(NdotV, NdotL, roughness);

            direct_color += ((diffuse+specular) * radiance * NdotL);
        }
        Vec3f indirect_color = 0.4f * albedo;
        //auto color(direct_color + indirect_color);
        //color = color / (color + Vec3f(1.0f, 1.0f, 1.0f));
        auto color = Exposure(direct_color+indirect_color, 0.8f);


        return TinyMath::Vec4f(ToGammar(color), 1.0f);
    }

    static Vec3f F(float costheta, Vec3f f0)
    {
        return f0 + (Vec3f(1.0f,1.0f,1.0f) - f0) * std::pow(1 - costheta, 5.0f);
    }
    static float NDF(float NdotH, float roughness)
    {
        //GGX
        const float pi = 3.1415926f;
        float a = roughness * roughness;
        float nom = a * a;
        float denom = (NdotH * NdotH * (nom - 1.0f) + 1.0f);
        denom = pi * denom * denom;
        return nom / denom;
    }
    static float Vis(float NdotV, float NdotL, float roughness)
    {
        float r = roughness + 1.0f;
        float k = (r * r) / 8.0f;

        float denom = 4 * (NdotV * (1 - k) + k) * (NdotL * (1 - k) + k);
        //prevent denom =0;
        return 1 / (denom+0.000001);
    }

public:

    //uniform 
    Mat4f u_mvp;
    Mat4f u_model;
    std::vector<Mat4f>* u_bones_matrix;
    Vec3f u_light_dir;
    Vec3f u_view_pos;
    Sampler u_albedo = 0;

    Sampler u_normal = 5;
    Sampler u_metalness_roughness = 6;
};



//using namespace TinyMath;
//template<typename Context = BlinnContext>
//class PbrShader :public Shader<Context>
//{
//public:
//    template<typename VAO>
//    TinyMath::Vec4f VertexShader(const VAO& vao, int index, Context& context) const
//    {
//        const VAO::VertexType& v = vao.get_vertex(index);
//
//
//        Mat4f transform = Mat4f::GetZero();
//        float a = 0.0f;
//        for (int i = 0; i < MAX_BOUNE_INFLUENCE; i++)
//        {
//            if (v.bone_ids_[i] == -1) continue;
//            int id = v.bone_ids_[i];
//            transform = transform + (*u_bones_matrix)[id] * v.weights_[i];
//            a += v.weights_[i];
//        }
//
//
//        Vec4f total_pos = transform * v.pos_;
//
//
//        context.o_coords_ = v.coords_;
//
//
//        context.o_tangent_ = u_model * transform* Vec4f(v.tangent_);
//        context.o_tangent_ = Normalize(context.o_tangent_);
//
//        context.o_bitangent_ = u_model * transform* Vec4f(v.bitangent_);
//        context.o_bitangent_ = Normalize(context.o_bitangent_);
//
//        context.o_normal_ = u_model * transform* Vec4f(v.normal_);
//        context.o_normal_ = Normalize(context.o_normal_);
//
//        context.o_coords_ = v.coords_;
//        context.o_pos_ = u_model * total_pos;
//        return u_mvp * total_pos;
//    }
//
//    TinyMath::Vec4f FragmentShader(const Context& context) const
//    {
//        //set data;
//        const float pi = 3.14158926f;
//        Vec3f normal = m_textures[u_normal]->Sampler2D(context.o_coords_);
//        normal = normal * 2.0f - Vec3f(1.0f, 1.0f, 1.0f);
//        Vec3f albedo = ToLinear(m_textures[u_albedo]->Sampler2D(context.o_coords_));
//        Vec3f metallic_roughness = m_textures[u_metalness_roughness]->Sampler2D(context.o_coords_);
//        float metallic = metallic_roughness.b_;
//        float roughness = metallic_roughness.g_;
//        static Mat3f TBN;
//        TBN.SetCol(0, context.o_tangent_);
//        TBN.SetCol(1, context.o_bitangent_);
//        TBN.SetCol(2, context.o_normal_);
//
//        const Vec3f light_color(1.0f, 1.0f, 1.0f);
//
//        static Vec3f N;
//        N = Normalize(TBN * normal);
//        auto& V = Normalize(u_view_pos - context.o_pos_);
//
//        Vec3f f0 = { 0.04f,0.04f,0.04f };
//        f0 = Mix(f0, albedo, metallic);
//
//        float NdotV = Between(0.0f, 1.0f, VectorDot(N, V));
//        Vec3f direct_color{ 0.0,0.0f,0.0f };
//        for (int i = 0; i < 1; i++)
//        {
//            //diffuse
//            const float dirlight_strength = 3.0f;
//            auto L = Normalize(context.o_pos_ - u_light_dir);
//            auto H = Normalize(L + V);
//
//
//            Vec3f radiance = light_color * dirlight_strength;
//
//            float NDF = DistributionGGX(N, H, roughness);
//            float G = GeometrySmith(N, V, L, roughness);
//            Vec3f F = fresnelSchlick(std::max(VectorDot(H, V), 0.0f), f0);
//
//
//
//            Vec3f numerator = NDF * G * F;
//            float denominator = 4.0 * std::max(VectorDot(N, V), 0.0f) * std::max(VectorDot(N, L), 0.0f) + 0.0001; // + 0.0001 to prevent divide by zero
//            Vec3f specular = numerator / denominator;
//
//            float kd = (1.0f - metallic);
//
//
//            float NdotL = std::max(VectorDot(N, L), 0.0f);
//
//            Vec3f diffuse = kd * albedo / pi;
//            direct_color += (( specular) * radiance * NdotL);
//        }
//        Vec3f indirect_color = 0.4f * albedo;
//
//        auto color = Exposure(direct_color+ indirect_color, 1.0f);
//
//        return TinyMath::Vec4f(direct_color, 1.0f);
//    }
//
//    static float DistributionGGX(Vec3f& N, Vec3f& H, float roughness)
//    {
//        const float PI = 3.1415926f;
//        float a = roughness * roughness;
//        float a2 = a * a;
//        float NdotH = std::max(VectorDot(N, H), 0.0f);
//        float NdotH2 = NdotH * NdotH;
//
//        float nom = a2;
//        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//        denom = PI * denom * denom;
//
//        return nom / denom;
//    }
//    // ----------------------------------------------------------------------------
//    static float GeometrySchlickGGX(float NdotV, float roughness)
//    {
//        float r = (roughness + 1.0);
//        float k = (r * r) / 8.0;
//
//        float nom = NdotV;
//        float denom = NdotV * (1.0 - k) + k;
//
//        return nom / denom;
//    }
//    // ----------------------------------------------------------------------------
//    static float GeometrySmith(const Vec3f& N, const Vec3f& V, const Vec3f& L, float roughness)
//    {
//        float NdotV = std::max(VectorDot(N, V), 0.0f);
//        float NdotL = std::max(VectorDot(N, L), 0.0f);
//        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
//        float ggx1 = GeometrySchlickGGX(NdotL, roughness);
//
//        return ggx1 * ggx2;
//    }
//    // ----------------------------------------------------------------------------
//    static Vec3f fresnelSchlick(float cosTheta, Vec3f F0)
//    {
//        return F0 + (Vec3f(1.0f,1.0f,1.0f) - F0) * std::pow(Between(0.0f, 1.0f, 1.0f - cosTheta), 5.0f);
//    }
//
//public:
//
//    //uniform 
//    Mat4f u_mvp;
//    Mat4f u_model;
//    std::vector<Mat4f>* u_bones_matrix;
//    Vec3f u_light_dir;
//    Vec3f u_view_pos;
//    Sampler u_albedo = 0;
//
//    Sampler u_normal = 5;
//    Sampler u_metalness_roughness = 6;
//};