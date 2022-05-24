#pragma once
#include"Math.h"
#include"Buffer.h"
#include"Texture.h"

#include<unordered_map>
#include"rttr/registration.h"
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

    template<typename VAO>
    TinyMath::Vec4f VertexShader(const VAO& vao, int index, void* context)const{}


    TinyMath::Vec4f FragmentShader(void* input)const{}

    static constexpr size_t get_context_count() {  return sizeof(Context)/sizeof(float); }
    float* get_input_context(int index,int thread) { return (float*)&m_input[thread][index]; }
    float* get_output_context(int thread) { return (float*)&m_output[thread]; }
    void SetTexture(const char* sampler, int index, Texture2D* tex) { 
        assert(index < 32);
        rttr::property prop = rttr::type::get(*this).get_property(sampler);
        prop.set_value(*this,index);
        m_textures[index] = tex; 
    }

protected:

    Context m_input[4][3];  //input memory;
    Context m_output[4];   //output memmory
    Texture2D* m_textures[32];


    //TODO: geometry shader;
};

