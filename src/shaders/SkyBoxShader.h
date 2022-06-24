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
		return u_mvp * v.pos_;
	}

	TinyMath::Vec4f FragmentShader(const Context* context) const
	{
		
		return ToGammar(Exposure(u_skybox->SamplerCube(context->dir_),1.0f));
	}
public:
	
	TinyMath::Mat4f u_model;
	TinyMath::Mat4f u_mvp;

	std::shared_ptr<TextureCube> u_skybox;
};
