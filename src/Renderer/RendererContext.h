#pragma once
#include<stdint.h>

#include"Math.h"
#include"FrameBuffer.h"

struct Vertex
{
	float* context_;
	float rhw_;
	TinyMath::Vec4f pos_;
	TinyMath::Vec2f center_;		//ÏñËØÖÐÐÄ float
	TinyMath::Vec2i coords_;		//ÏñËØ×ø±ê
};

struct TaskInfo
{
	std::vector<uint32_t> triangle_ids_;
	int trangle_count = 0;
};
struct TriangleContext
{
	Vertex vs_[3];
	Vertex* ordered_vs_[3];
	bool top_left01_;
	bool top_left12_;
	bool top_left20_;
	bool tag_ = false;
	int min_x_, max_x_, min_y_, max_y_;

};
//class ContextCache
//{
//public:
//	ContextCache() = default;
//	ContextCache(uint32_t width, uint32_t height)
//		:m_width(width),m_height(height)
//	{
//		m_data = new float[m_width * m_height * m_count];
//	}
//
//
//	~ContextCache() { if (m_data) delete[] m_data; }
//private:
//	float* m_data=nullptr;
//	uint32_t m_count = 16;
//	uint32_t m_width = 0;
//	uint32_t m_height = 0;
//};
//
//class RenderContext
//{
//public:
//	RenderContext() = default;
//	RenderContext(uint32_t width, uint32_t height);
//
//
//private:
//	Color m_color_fg={0xff,0xff,0xff,0xff};
//	Color m_color_bg = { 0x00,0x00,0x00,0xff };
//
//
//
//
//	
//	//size:trangles*3*data_size,data_size is dynamic
//	float* m_input_context;
//	FrameBuffer* m_default_buffer;
//	uint32_t m_width;
//	uint32_t m_height;
//	
//};