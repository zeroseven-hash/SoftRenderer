#pragma once
#include<stdint.h>

#include"Math.h"
#include"FrameBuffer.h"

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
	Vertex vs_[3];
	Vertex* ordered_vs_[3];
	/*bool top_left01_;
	bool top_left12_;
	bool top_left20_;*/
	bool tag_ = false;
	int min_x_, max_x_, min_y_, max_y_;

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
