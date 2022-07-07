#pragma once
#include"Renderer/Shader.h"
#include"Renderer/TextureCube.h"
#include"PbrShader.h"

#include<memory>

template<typename Context = BlinnContext>
class IblShader :public Shader<Context>
{
public:
	template<typename VAO>
	TinyMath::Vec4f VertexShader(const VAO& vao, int index, Context& context)const
	{
		const auto& v = vao.get_vertex(index);
		context.o_normal_ = u_model * TinyMath::Vec4f(v.normal_);
		context.o_pos_ = u_model * v.pos_;


		context.o_tangent_ = u_model  * Vec4f(v.tangent_);
		context.o_tangent_ = Normalize(context.o_tangent_);

		context.o_bitangent_ = u_model  * Vec4f(v.bitangent_);
		context.o_bitangent_ = Normalize(context.o_bitangent_);

		context.o_normal_ = u_model  * Vec4f(v.normal_);
		context.o_normal_ = Normalize(context.o_normal_);

		context.o_coords_ = v.coords_;
		context.o_pos_ = u_model * v.pos_;
		return u_mvp * v.pos_;
	}

	TinyMath::Vec4f FragmentShader(const Context* context, const Context delta[2]) const
	{
		float ddx = delta[0].o_coords_.x_*m_textures[u_albedo]->get_width();
		float ddy = delta[0].o_coords_.y_* m_textures[u_albedo]->get_height();
		float lod = CalLod(TinyMath::Vec2f(ddx, ddy));


		Vec3f normal = m_textures[u_normal]->Sampler2DLod(context[0].o_coords_,lod);
		normal = normal * 2.0f - Vec3f(1.0f, 1.0f, 1.0f);
		float ao = m_textures[u_ao]->Sampler2DLod(context[0].o_coords_,lod).x_;

		Vec3f emissive = m_textures[u_emmisive]->Sampler2DLod(context[0].o_coords_,lod);
		Vec3f albedo = ToLinear(m_textures[u_albedo]->Sampler2DLod(context[0].o_coords_,lod));
		Vec3f metallic_roughness = m_textures[u_metalness_roughness]->Sampler2DLod(context[0].o_coords_,lod);
		float metallic = metallic_roughness.b_;
		float roughness = metallic_roughness.g_;
		static Mat3f TBN;
		TBN.SetCol(0, context[0].o_tangent_);
		TBN.SetCol(1, context[0].o_bitangent_);
		TBN.SetCol(2, context[0].o_normal_);
		static Vec3f N;
		N = Normalize(TBN * normal);
		auto V = Normalize(u_view_pos - context[0].o_pos_);
		auto R = TinyMath::Reflect(-V, N);

		float NdotV = TinyMath::Between(0.0f, 1.0f, VectorDot(N, V));
		TinyMath::Vec3f irrdiance = u_skybox_irr->SamplerCube(N);
		Vec3f diffuse = irrdiance * albedo;

		Vec3f f0 = { 0.04f,0.04f,0.04f };
		f0 = Mix(f0, albedo, metallic);

		Vec3f F = FresnelSchlickRoughness(NdotV, f0, roughness);
		Vec3f  kd = (1 - metallic)*(Vec3f(1.0f,1.0f,1.0f)-F);
		const float MAX_LOD = 2.0f;

		Vec3f prefilterColor = u_skybox_spec->SamplerCubeLod(R, roughness * MAX_LOD);
		Vec2f brdf = m_textures[u_brdf]->Sampler2D({ NdotV, 0.999f*roughness });
		Vec3f specular = prefilterColor * (F * brdf.x_ + brdf.y_);
		
		Vec3f ambient = emissive+(kd*diffuse+specular)*ao;
		return ToGammar(Exposure(ambient, 2.0f));
	}

	static TinyMath::Vec3f FresnelSchlickRoughness(float cos, const TinyMath::Vec3f& f0, float rough)
	{
		TinyMath::Vec3f temp = {
			std::max(1.0f - rough,f0.x_) - f0.x_,
			std::max(1.0f - rough,f0.y_) - f0.y_,
			std::max(1.0f - rough,f0.z_) - f0.z_
		};
		return f0 + temp * std::pow(1.0f-cos, 5.0f);
	}
public:

	TinyMath::Mat4f u_model;
	TinyMath::Mat4f u_mvp;
	Vec3f u_view_pos;

	
	std::shared_ptr<TextureCube> u_skybox;
	std::shared_ptr<TextureCube> u_skybox_irr;
	std::shared_ptr<TextureCube> u_skybox_spec;
	Sampler u_albedo = 0;
	Sampler u_normal = 5;
	Sampler u_metalness_roughness = 6;
	Sampler u_emmisive = 4;
	Sampler u_ao = 7;
	Sampler u_brdf = 8;
};
