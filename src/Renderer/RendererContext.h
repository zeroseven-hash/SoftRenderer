#pragma once
#include<stdint.h>

#include"Math.h"
#include"FrameBuffer.h"
const int MAXVERTICES = 10;
struct Vertex
{
	float* context_;
	float rhw_;
	TinyMath::Vec4f pos_;
	TinyMath::Vec3f ndc_coords_; //ndc 坐标
	TinyMath::Vec2i inter_coords; //整数坐标
};


struct TriangleContext
{
	Vertex input_vs_[MAXVERTICES];
	Vertex output_vs_[MAXVERTICES];
	uint32_t vertices_nums;
	bool tag_ = false;
	bool swap_ = false;
};
template<typename T>
struct CacheBlock
{
public:
	CacheBlock() = default;
	CacheBlock(uint32_t length, uint32_t count)
		:length_(length), count_(count)
	{
		data_ = new T[length * count];
	}

	void Resize(uint32_t length, uint32_t count)
	{
		if (data_) delete data_;
		length_ = length;
		count_ = count;
		data_ = new T[length * count];
	}

	T& operator[](size_t index) { return data_[index]; }
	const T& operator[](size_t index)const { return data_[index]; }
	~CacheBlock() { if (data_) delete[] data_; }

public:
	uint32_t length_;
	uint32_t count_;

	T* data_;
};


struct RendererContext
{
	~RendererContext()
	{
		if (default_buffer_) delete default_buffer_;
	}
	Color color_fg_ = { 0xff,0xff,0xff,0xff };
	Color color_bg_ = { 0x00,0x00,0x00,0xff };
	
	FrameBuffer* default_buffer_;
	uint32_t width_;
	uint32_t height_;

	CacheBlock<float> input_shadercontexts_;
	CacheBlock<TriangleContext> triangles_;

	int renderer_flag;
};
