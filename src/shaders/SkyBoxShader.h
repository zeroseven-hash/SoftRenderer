#pragma once
#include"Renderer/Shader.h"
#include"Renderer/TextureCube.h"

#include<memory>

struct CubeContext
{
	Vec3f dir_;
};
template<typename Context= CubeContext>
class SkyBoxShader:public Shader<Context>
{
public:
	template<typename VAO>
	TinyMath::Vec4f VertexShader(const VAO& vao, int index, Context& context)const
	{
		const auto& v = vao.get_vertex(index);
		context.dir_ = v.pos_;
		auto pos = u_mvp * v.pos_;

		return TinyMath::Vec4f(pos.x_,pos.y_,pos.w_*0.999f,pos.w_);
	}

	TinyMath::Vec4f FragmentShader(const Context* context, const Context delta[2]) const
	{
		
		return ToGammar(Exposure(u_skybox->SamplerCube(context->dir_),2.0f));
	}
public:
	
	TinyMath::Mat4f u_model;
	TinyMath::Mat4f u_mvp;
	std::shared_ptr<TextureCube> u_skybox;
};
