#pragma
#include<stdint.h>
#include<memory>
#include<map>
#include<cstring>
#include<functional>

#include"Math.h"
#include"Buffer.h"
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

typedef std::function<TinyMath::Vec4f(int index, ShaderContext&)> VertexShader;
typedef std::function<TinyMath::Vec4f(ShaderContext&)> FragmentShader;

struct Shader
{
	VertexShader vertex_shader_;
	FragmentShader fragment_shader_;
	//TODO: geometry shader;
};

struct Color
{
	uint8_t r_ = 0, g_ = 0, b_ = 0, a_ = 0;
	Color() = default;
	Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) :r_(r), g_(g), b_(b), a_(a) {}
	Color(const TinyMath::Vec4f& color)
	{
		r_ = (uint8_t)TinyMath::Between(0, 255, (int)(color.r_ * 255));
		g_ = (uint8_t)TinyMath::Between(0, 255, (int)(color.g_ * 255));
		b_ = (uint8_t)TinyMath::Between(0, 255, (int)(color.b_ * 255));
		a_ = (uint8_t)TinyMath::Between(0, 255, (int)(color.a_ * 255));
	}
	TinyMath::Vec4f TransformToVec()
	{
		TinyMath::Vec4f res;
		res.r_ = TinyMath::Between(0.0f, 1.0f, (float)this->r_ / 255);
		res.g_ = TinyMath::Between(0.0f, 1.0f, (float)this->g_ / 255);
		res.b_ = TinyMath::Between(0.0f, 1.0f, (float)this->b_ / 255);
		res.a_ = TinyMath::Between(0.0f, 1.0f, (float)this->a_ / 255);

		
		return res;
	}
};
struct Vertex
{
	ShaderContext context_;
	float rhw_;
	TinyMath::Vec4f pos_;
	TinyMath::Vec2f center_;		//像素中心 float
	TinyMath::Vec2i coords_;		//像素坐标
};


class Canvas
{
public:
	inline Canvas(const char* filepath) { LoadFile(filepath); }
	inline Canvas(int width, int height) :m_width(width), m_height(height), m_channel(4)
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
public:
	
	inline void Fill(const Color& color)
	{
		for (int i = 0; i < m_height; i++)
		{
			uint8_t* row = (uint8_t*)(m_bits + i * m_channel * m_width);
			for (int j = 0; j < m_width; j++, row+=4)
				memcpy(row, &color, sizeof(Color));
		}
	}
	void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
	void LoadFile(const char* filename);
	void SaveFileBMP(const char* filename);
	Color Sample2D(const TinyMath::Vec2f& tex_coords);
	static inline Color LinearInterp(const Color& a, const Color& b, float t)
	{
		Color c;
		c.r_ = a.r_ + (b.r_ - a.r_) * t;
		c.g_ = a.g_ + (b.g_ - a.g_) * t;
		c.b_ = a.b_ + (b.b_ - a.b_) * t;
		c.a_ = a.a_ + (b.a_ - a.a_) * t;
		return c;
	}
public:
	inline void set_pixel(int x, int y, const Color& color)
	{
		if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		{
			memcpy(m_bits + y * m_width * m_channel + x * m_channel, &color, sizeof(Color));
		}
	}

	inline Color get_pixel(int x, int y)
	{
		Color color;
		if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		{
			memcpy(&color, m_bits + y * m_width * m_channel + x * m_channel, sizeof(color));
		}
		return color;
	}
	inline uint32_t get_pixel1(int x,int y)
	{
		uint32_t color;
		if (x >= 0 && x < m_width && y >= 0 && y < m_height)
		{
			memcpy(&color, m_bits + y * m_width * m_channel + x * m_channel, sizeof(uint32_t));
		}
		return color;
	}
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
	void DrawTriangle(const Shader& shader,uint32_t* indices);
	template<typename T,typename U>
	inline void DrawArray(const VertexArrayBuffer<T, U>& vao, const Shader& shader)
	{
		const auto& vs = vao.get_vertices();
		const auto& indices = vao.get_indices();
		if (!vs.size() || !indices.size())return;
		for (int i = 0;i < indices.size();i += 3)
		{
			uint32_t is[3] = { (uint32_t)indices[i],(uint32_t)indices[i + 1],(uint32_t)indices[i + 2] };
			DrawTriangle(shader, is);
		}
	}
public:
	inline int get_width()const  { return m_width; }
	inline int get_height()const  { return m_height; }
	inline uint8_t* get_canvas()const  { return m_canvas->get_bits(); }
	inline void set_render_state(bool pixel, bool line)
	{
		m_render_pixel = pixel;
		m_render_line = line;
	}
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


	bool m_render_line;
	bool m_render_pixel;
};