#pragma once
#include<stdint.h>
#include<memory>
#include<map>
#include<cstring>
#include<functional>
#include<thread>
//#include<omp.h>

#include"Math.h"
#include"Buffer.h"
#include"Shader.h"
typedef int BufferFlag;

enum BufferFlag_
{
	DEPTH_BUFFER_BIT=Bit(0),
	COLOR_BUFFER_BIT=Bit(1)
};



struct Color
{
	uint8_t r_ = 0, g_ = 0, b_ = 0, a_ = 0;
	Color() = default;
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :r_(r), g_(g), b_(b), a_(a) {}
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
	Canvas() = default;
	Canvas(const char* filepath) { LoadFile(filepath); }
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
	void Init(int width,int height);
	void Clear(BufferFlag flag);



	template<typename VAO, typename SHADER>
	void DrawTriangle(const VAO& vao, SHADER& shader, uint32_t* indices)
	{
		if (m_canvas == nullptr) return;
		for (int k = 0;k < 3;k++)
		{
			Vertex& v = m_vertex[k];
			v.context_.varying_float_.clear();
			v.context_.varying_vec2f_.clear();
			v.context_.varying_vec3f_.clear();
			v.context_.varying_vec4f_.clear();

			v.pos_ = shader.VertexShader(vao, indices[k], v.context_);

			//裁剪 pos(nx,ny,n^2,z);x y should between [-1,1],n could be near or far

			//w_>0;
			float w = v.pos_.w_;
			if (w == 0.0f)return;

			if (v.pos_.z_<-w || v.pos_.z_>w)return;
			if (v.pos_.x_<-w || v.pos_.x_>w)return;
			if (v.pos_.y_<-w || v.pos_.y_>w)return;


			v.rhw_ = 1.0f / w;
			v.pos_ *= v.rhw_;

			//x:[-1,1] to [0,width-1],y:[-1,1] to [0,height-1];
			//	0.5*width       0	      width*0.5		    x     (x+1.0f)*0.5*width
			//	  0	       height*0.5    height*0.5    *    y  = (y+1.0f)*0.5*height
			//	  0             0            0	        	1 				1
			v.center_.x_ = (v.pos_.x_ + 1.0f) * m_width * 0.5f;
			v.center_.y_ = (v.pos_.y_ + 1.0f) * m_height * 0.5f;


			v.coords_.x_ = (int)(v.center_.x_ + 0.5f);
			v.coords_.y_ = (int)(v.center_.y_ + 0.5f);

			if (k == 0)
			{
				m_min_x = m_max_x = TinyMath::Between(0, m_width - 1, v.coords_.x_);
				m_min_y = m_max_y = TinyMath::Between(0, m_height - 1, v.coords_.y_);
			}
			else
			{
				m_min_x = TinyMath::Between(0, m_width - 1, std::min(m_min_x, v.coords_.x_));
				m_max_x = TinyMath::Between(0, m_width - 1, std::max(m_max_x, v.coords_.x_));
				m_min_y = TinyMath::Between(0, m_height - 1, std::min(m_min_y, v.coords_.y_));
				m_max_y = TinyMath::Between(0, m_height - 1, std::max(m_max_y, v.coords_.y_));
			}
		}
		//绘制线框
		if (m_render_line)
		{
			m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
			m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_color_fg);
			m_canvas->DrawLine(m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		}
		if (!m_render_pixel) return;

		TinyMath::Vec4f v01 = m_vertex[1].pos_ - m_vertex[0].pos_;
		TinyMath::Vec4f v02 = m_vertex[2].pos_ - m_vertex[0].pos_;
		TinyMath::Vec4f normal = v01.Cross(v02);

		Vertex* vs[3] = { &m_vertex[0],&m_vertex[1] ,&m_vertex[2] };
		if (normal.z_ < 0.0f)
			std::swap(vs[1], vs[2]);
		else if (normal.z_ == 0.0f)
			return;

		TinyMath::Vec2i p[3] = { vs[0]->coords_,vs[1]->coords_ ,vs[2]->coords_ };

		// 计算面积，为零就退出
		float s = std::abs((p[1] - p[0]).Cross(p[2] - p[0]));
		if (s <= 0) return;

		bool TopLeft01 = IsTopLeft(p[0], p[1]);
		bool TopLeft12 = IsTopLeft(p[1], p[2]);
		bool TopLeft20 = IsTopLeft(p[2], p[0]);

		ShaderContext input[2];

		auto func = [&](int x_min, int x_max,ShaderContext& input)
		{
			for (int x = x_min; x <= x_max; x++) for (int y = m_min_y; y <= m_max_y; y++)
			{
				TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };

				// Edge Equation
				// 使用整数避免浮点误差，同时因为是左手系，所以符号取反
				int E01 = -(x - p[0].x_) * (p[1].y_ - p[0].y_) + (y - p[0].y_) * (p[1].x_ - p[0].x_);
				int E12 = -(x - p[1].x_) * (p[2].y_ - p[1].y_) + (y - p[1].y_) * (p[2].x_ - p[1].x_);
				int E20 = -(x - p[2].x_) * (p[0].y_ - p[2].y_) + (y - p[2].y_) * (p[0].x_ - p[2].x_);


				// 如果是左上边，用 E >= 0 判断合法，如果右下边就用 E > 0 判断合法
				// 这里通过引入一个误差 1 ，来将 < 0 和 <= 0 用一个式子表达
				if (E01 < (TopLeft01 ? 0 : 1)) return;   // 在第一条边后面
				if (E12 < (TopLeft12 ? 0 : 1)) return;   // 在第二条边后面
				if (E20 < (TopLeft20 ? 0 : 1)) return;   // 在第三条边后面


				// 三个端点到当前点的矢量
				TinyMath::Vec2f s0 = vs[0]->center_ - pixel;
				TinyMath::Vec2f s1 = vs[1]->center_ - pixel;
				TinyMath::Vec2f s2 = vs[2]->center_ - pixel;

				// 重心坐标系：计算内部子三角形面积 a / b / c
				float a = std::abs(s1.Cross(s2));    // 子三角形 Px-P1-P2 面积
				float b = std::abs(s2.Cross(s0));    // 子三角形 Px-P2-P0 面积
				float c = std::abs(s0.Cross(s1));    // 子三角形 Px-P0-P1 面积
				float s = a + b + c;                 // 大三角形 P0-P1-P2 面积

				if (s == 0.0f) return;

				// Barycentric coordinates interpolation
				a = a * (1.0f / s);
				b = b * (1.0f / s);
				c = c * (1.0f / s);

				//深度以及其倒数在可以直接使用重心插值
				float rhw = vs[0]->rhw_ * a + vs[1]->rhw_ * b + vs[2]->rhw_ * c;

				//depth_buffer数组下标和x,y相反
				if (rhw < m_depth_buffer[y][x])return;
				m_depth_buffer[y][x] = rhw;

				float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

				//透视正确插值
				float c0 = vs[0]->rhw_ * a * w;
				float c1 = vs[1]->rhw_ * b * w;
				float c2 = vs[2]->rhw_ * c * w;


				ShaderContext& i0 = vs[0]->context_;
				ShaderContext& i1 = vs[1]->context_;
				ShaderContext& i2 = vs[2]->context_;


				// 插值各项 varying
				for (auto const& it : i0.varying_float_) {
					int key = it.first;
					float f0 = i0.varying_float_[key];
					float f1 = i1.varying_float_[key];
					float f2 = i2.varying_float_[key];
					input.varying_float_[key] = c0 * f0 + c1 * f1 + c2 * f2;
				}

				for (auto const& it : i0.varying_vec2f_) {
					int key = it.first;
					const TinyMath::Vec2f& f0 = i0.varying_vec2f_[key];
					const TinyMath::Vec2f& f1 = i1.varying_vec2f_[key];
					const TinyMath::Vec2f& f2 = i2.varying_vec2f_[key];
					input.varying_vec2f_[key] = c0 * f0 + c1 * f1 + c2 * f2;
				}

				for (auto const& it : i0.varying_vec3f_) {
					int key = it.first;
					const TinyMath::Vec3f& f0 = i0.varying_vec3f_[key];
					const TinyMath::Vec3f& f1 = i1.varying_vec3f_[key];
					const TinyMath::Vec3f& f2 = i2.varying_vec3f_[key];
					input.varying_vec3f_[key] = c0 * f0 + c1 * f1 + c2 * f2;
				}

				for (auto const& it : i0.varying_vec4f_) {
					int key = it.first;
					const TinyMath::Vec4f& f0 = i0.varying_vec4f_[key];
					const TinyMath::Vec4f& f1 = i1.varying_vec4f_[key];
					const TinyMath::Vec4f& f2 = i2.varying_vec4f_[key];
					input.varying_vec4f_[key] = c0 * f0 + c1 * f1 + c2 * f2;
				}

				TinyMath::Vec4f color = shader.FragmentShader(input);
				m_canvas->set_pixel(x, y, Color(color));
			}
			
		};

		
//#pragma omp parallel for firstprivate(input) schedule(static,200) 
		int offset = (m_max_x - m_min_x)/2;
		static std::thread threads[2];
		threads[0] = std::thread(func, m_min_x, m_min_x + offset, input[0]);
		threads[1] = std::thread(func, m_min_x + offset + 1, m_max_x,input[1]);
		/*std::thread threads0(func,m_min_x,m_min_x+offset);
		std::thread threads1(func, m_min_x + offset + 1, m_max_x);*/

		threads[0].join();
		threads[1].join();

		/*for (int x = m_min_x;x <= m_max_x;x++) for (int y = m_min_y;y <= m_max_y;y++)
		{
			
		}*/

		if (m_render_line)
		{
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[2]->coords_.x_, vs[2]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[2]->coords_.x_, vs[2]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
		}

	}


	template<typename VAO, typename SHADER>
	inline void DrawArray(const VAO& vao, SHADER& shader)
	{
		const auto& vs = vao.get_vertices();
		const auto& indices = vao.get_indices();
		if (!vs.size() || !indices.size())return;
		for (int i = 0;i < indices.size();i += 3)
		{
			uint32_t is[3] = { (uint32_t)indices[i],(uint32_t)indices[i + 1],(uint32_t)indices[i + 2] };
			DrawTriangle(vao,shader, is);
		}
	}
public:
	inline int get_width()const  { return m_width; }
	inline int get_height()const  { return m_height; }
	inline uint8_t* get_canvas()const { return m_canvas->get_bits(); }
	inline void set_line_color(const Color& color) { m_color_fg = color; }
	inline void set_clear_color(const Color& color) { m_color_bg = color; }
	inline void set_render_state(bool pixel, bool line)
	{
		m_render_pixel = pixel;
		m_render_line = line;
	}
protected:
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