#pragma once
#include"Math.h"
#include"Buffer.h"
#include"Texture.h"

#include<unordered_map>
#include<functional>
//only supprt
struct ShaderContextDefault
{
    TinyMath::Vec2f o_coords;
    TinyMath::Vec4f o_color_;
};




typedef int Sampler;
using FragmentShader = std::function<TinyMath::Vec4f(void* input)>;
class Canvas;
template<typename Context=ShaderContextDefault>
class Shader
{
public:
    using ContextType = Context;


    //vertex shader
    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, Context& context)const{}

    //fragmentshader
    TinyMath::Vec4f FragmentShader(const Context* input,const Context delta[2])const {}

    void SetTexture(int index, Texture2D* tex) { 
        assert(index < 32);
        m_textures[index] = tex; 
    }

protected:
    Texture2D* m_textures[32];


    //TODO: geometry shader;
};

//some shader function
TinyMath::Vec4f ToLinear(const TinyMath::Vec4f& color);
TinyMath::Vec4f ToGammar(const TinyMath::Vec4f& color);
TinyMath::Vec3f ToGammar(const TinyMath::Vec3f& color);
TinyMath::Vec3f Exposure(const TinyMath::Vec3f& color,float exposure);
float CalLod(const TinyMath::Vec2f& dxdy);