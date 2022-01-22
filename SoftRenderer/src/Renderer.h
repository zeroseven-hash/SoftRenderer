#pragma
#include<stdint.h>
#include<memory>
#include<map>
#include<functional>

#include"Math.h"

typedef int BufferFlag;
enum BufferFlag_
{
	DEPTH_BUFFER_BIT=Bit(0),
	COLOR_BUFFER_BIT=Bit(1)
};
struct ShaderContext
{
	std::map<int, float> varying_float_;    // 浮点数 varying 列表
	std::map<int, TinyMath::Vec2f> varying_vec2f_;    // 二维矢量 varying 列表
	std::map<int, TinyMath::Vec3f> varying_vec3f_;    // 三维矢量 varying 列表
	std::map<int, TinyMath::Vec4f> varying_vec4f_;    // 四维矢量 varying 列表
};

struct Color
{
	uint8_t r_ = 0, g_ = 0, b_ = 0, a_ = 0;
	Color() = default;
	Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) :r_(r), g_(g), b_(b), a_(a) {}
};
struct Vertex
{
	ShaderContext context_;
	float rhw_;
	TinyMath::Vec4f pos_;
	TinyMath::Vec2f center;		//像素中心 float
	TinyMath::Vec2i coords;		//像素坐标
};

typedef std::function<TinyMath::Vec4f(int index, ShaderContext&)> VertexShader;
typedef std::function<TinyMath::Vec4f(ShaderContext&)> FragmentShader;


class Canvas
{
public:
	inline Canvas(int width, int height) :m_width(width), m_height(height),m_channel(4)
	{
		m_bits = new uint8_t[m_width * m_channel * m_height];
		Fill(Color(0,0,0,0));
	}
	inline Canvas(const Canvas& canvas) : m_width(canvas.m_width), m_height(canvas.m_height), m_channel(canvas.m_channel)
	{
		m_bits = new uint8_t[m_width * m_channel * m_height];
		memcpy(m_bits, canvas.m_bits, m_width * m_channel * m_height);
	}
	inline ~Canvas() { if (m_bits)delete[] m_bits; m_bits = nullptr; }

	inline void Fill(Color color)
	{
		for (int i = 0; i < m_height; i++)
		{
			uint8_t* row = (uint8_t*)(m_bits + i * m_channel * m_width);
			for (int j = 0; j < m_width; j++, row+=4)
				memcpy(row, &color, sizeof(Color));
		}
	}

public:
	uint8_t* get_bits() { return m_bits; }
private:
	int32_t m_width;
	int32_t m_height;
	int32_t m_channel;
	uint8_t* m_bits;
};



class Renderer
{
public:
	inline Renderer()
	{
		m_canvas = nullptr;
		m_depth_buffer = NULL;
		m_render_line = false;
		m_render_pixel = true;
	}

	inline Renderer(int width, int height)
	{
		m_canvas = nullptr;
		m_depth_buffer = NULL;
		m_render_line = false;
		m_render_pixel = true;
		Init(width, height);
		
	}
	inline ~Renderer(){}
public:
	void Clear(BufferFlag flag);

public:
	int get_width() { return m_width; }
	int get_height() { return m_width; }
	uint8_t* get_canvas() { return m_canvas->get_bits(); }
protected:
	void Init(int width,int height);
	void Reset();
protected:
	Canvas* m_canvas;
	float** m_depth_buffer;

	Color m_color_fg;
	Color m_color_bg;
	
	int m_width;
	int m_height;

	Vertex m_vertex[3];
	int m_min_x;
	int m_min_y;
	int m_max_x;
	int m_max_y;


	VertexShader m_vertex_shader;
	FragmentShader m_fragment_shader;

	bool m_render_line;
	bool m_render_pixel;
};