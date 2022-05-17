#pragma once
#include"Math.h"
#include"Buffer.h"

#include<unordered_map>

#include"rttr/registration.h"
struct ShaderContext
{
    std::unordered_map<int, float> varying_float_;    // 浮点数 varying 列表
    std::unordered_map<int, TinyMath::Vec2f> varying_vec2f_;    // 二维矢量 varying 列表
    std::unordered_map<int, TinyMath::Vec3f> varying_vec3f_;    // 三维矢量 varying 列表
    std::unordered_map<int, TinyMath::Vec4f> varying_vec4f_;    // 四维矢量 varying 列表
};


typedef int Sampler;

class Canvas;
class Shader
{
public:

    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, ShaderContext& context)const{}


    TinyMath::Vec4f FragmentShader(ShaderContext& context)const{}

    ShaderContext& get_context() { return m_context;}
    void SetTexture(const char* sampler, int index, Canvas* tex) { 
        assert(index < 32);
        rttr::property prop = rttr::type::get(*this).get_property(sampler);
        prop.set_value(*this,index);
        m_textures[index] = tex; 
    }

protected:
    ShaderContext m_context;
    Canvas* m_textures[32];


    //TODO: geometry shader;
};

