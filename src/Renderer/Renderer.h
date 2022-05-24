#pragma once
#include<stdint.h>
#include<memory>
#include<map>
#include<cstring>
#include<functional>
#include<thread>

#include"Math.h"
#include"Buffer.h"
#include"Shader.h"
#include"Timer.h"
#include"Texture.h"
#include"FrameBuffer.h"
#include"Thread/ThreadPool.h"
typedef int BufferFlag;
typedef int RenderFlag;

enum BufferFlag_
{
	DEPTH_BUFFER_BIT=BIT(0),
	COLOR_BUFFER_BIT=BIT(1)
};

enum RenderFlag_
{
	FACE_CULL=BIT(0),
	DRAW_PIXEL=BIT(1),
	DRAW_LINE=BIT(2)
};


struct alignas(64) Vertex
{
	float* context_;
	float rhw_;
	TinyMath::Vec4f pos_;
	TinyMath::Vec2f center_;		//像素中心 float
	TinyMath::Vec2i coords_;		//像素坐标
};



class Renderer
{
public:
	Renderer() = delete;
	inline ~Renderer() { if (m_default_buffer) delete m_default_buffer; }
public:
	static void Init(uint32_t width,uint32_t height);
	static void Clear(BufferFlag flag,const Color& color=Color(0x00,0x00,0x00,0x00d),float depth=1.0f);
	static void Bind(FrameBuffer* fb);
	static void UnBind();
	static void SetViewPort(uint32_t width, uint32_t height);
	static void LineColor(const Color& color) { m_color_fg = color; }
	static void SetState(RenderFlag_ flag) { m_render_flag = flag; }
	static void FlushFrame();
	static void FlushFrame(FrameBuffer* fb, int attachment);
	static void Destory()
	{
		if (m_default_buffer) delete m_default_buffer;
		for (int i = 0; i < 4; i++)
		{
			if (m_temp_buffer[i]) delete m_temp_buffer[i];
		}
		m_thread_pool.Stop();
	}
	template<typename VAO, typename SHADER>
	static void DrawTriangle(const VAO& vao, SHADER& shader, uint32_t* indices,int theard_num)
	{
		int min_x = 0, max_x = 0;
		int min_y = 0, max_y = 0;
		for (int k = 0;k < 3;k++)
		{
			Vertex& v = m_vertex[theard_num][k];
			
			v.context_ = shader.get_input_context(k, theard_num);
			v.pos_ = shader.VertexShader(vao, indices[k], v.context_);

			//裁剪 pos(nx,ny,n^2,z);x y should between [-1,1],n could be near or far

			//w_>0;
			float w = v.pos_.w_;
			if (w == 0.0f)   return; 

			if (v.pos_.x_<-w || v.pos_.x_>w)  return;
			if (v.pos_.y_<-w || v.pos_.y_>w)  return;
			if (v.pos_.z_<-w || v.pos_.z_>w)  return;


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
				min_x = max_x = TinyMath::Between(0, m_width - 1, v.coords_.x_);
				min_y = max_y = TinyMath::Between(0, m_height - 1, v.coords_.y_);
			}
			else
			{
				min_x = TinyMath::Between(0, m_width - 1, std::min(min_x, v.coords_.x_));
				max_x = TinyMath::Between(0, m_width - 1, std::max(max_x, v.coords_.x_));
				min_y = TinyMath::Between(0, m_height - 1, std::min(min_y, v.coords_.y_));
				max_y = TinyMath::Between(0, m_height - 1, std::max(max_y, v.coords_.y_));
			}
		}

	
		////绘制线框
		//if (m_render_flag&DRAW_LINE)
		//{
		//	m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		//	m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_color_fg);
		//	m_canvas->DrawLine(m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		//}
		if (!(m_render_flag&DRAW_PIXEL)) return;

		TinyMath::Vec4f v01 = m_vertex[theard_num][1].pos_ - m_vertex[theard_num][0].pos_;
		TinyMath::Vec4f v02 = m_vertex[theard_num][2].pos_ - m_vertex[theard_num][0].pos_;
		TinyMath::Vec4f normal = v01.Cross(v02);

		//back face cull
		Vertex* vs[3] = { &m_vertex[theard_num][0],&m_vertex[theard_num][1] ,&m_vertex[theard_num][2]};
		
		if (m_render_flag & FACE_CULL)
		{
			if (normal.z_ <= 0.0) return;
		}
		else
		{
			if (normal.z_ < 0.0f)
				std::swap(vs[1], vs[2]);
			else if (normal.z_ == 0.0f)
				return;
		}

		TinyMath::Vec2i p[3] = { vs[0]->coords_,vs[1]->coords_ ,vs[2]->coords_ };

		// 计算面积，为零就退出
		int si = std::abs((p[1] - p[0]).Cross(p[2] - p[0]));
		if (si <= 0) return;

		bool TopLeft01 = IsTopLeft(p[0], p[1]);
		bool TopLeft12 = IsTopLeft(p[1], p[2]);
		bool TopLeft20 = IsTopLeft(p[2], p[0]);
		float* i0 = vs[0]->context_;
		float* i1 = vs[1]->context_;
		float* i2 = vs[2]->context_;

//#pragma omp parallel for schdule(static,200)
		for (int y = min_y; y <= max_y; y++)
		{
			for (int x = min_x; x <= max_x; x++)
			{
				// Edge Equation
				// 使用整数避免浮点误差，同时因为是左手系，所以符号取反
				int E01 = -(x - p[0].x_) * (p[1].y_ - p[0].y_) + (y - p[0].y_) * (p[1].x_ - p[0].x_);
				int E12 = -(x - p[1].x_) * (p[2].y_ - p[1].y_) + (y - p[1].y_) * (p[2].x_ - p[1].x_);
				int E20 = -(x - p[2].x_) * (p[0].y_ - p[2].y_) + (y - p[2].y_) * (p[0].x_ - p[2].x_);


				// 如果是左上边，用 E >= 0 判断合法，如果右下边就用 E > 0 判断合法
				// 这里通过引入一个误差 1 ，来将 < 0 和 <= 0 用一个式子表达
				if (E01 < (TopLeft01 ? 0 : 1)) continue;   // 在第一条边后面
				if (E12 < (TopLeft12 ? 0 : 1)) continue;   // 在第二条边后面
				if (E20 < (TopLeft20 ? 0 : 1)) continue;   // 在第三条边后面


				// 三个端点到当前点的矢量
				TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };
				TinyMath::Vec2f s0 = vs[0]->center_ - pixel;
				TinyMath::Vec2f s1 = vs[1]->center_ - pixel;
				TinyMath::Vec2f s2 = vs[2]->center_ - pixel;

				// 重心坐标系：计算内部子三角形面积 a / b / c
				float a = std::abs(s1.Cross(s2));    // 子三角形 Px-P1-P2 面积
				float b = std::abs(s2.Cross(s0));    // 子三角形 Px-P2-P0 面积
				float c = std::abs(s0.Cross(s1));    // 子三角形 Px-P0-P1 面积
				float s = a + b + c;                 // 大三角形 P0-P1-P2 面积

				if (s == 0.0f) continue;

				// Barycentric coordinates interpolation
				a /= s;
				b /= s;
				c /= s;

				//深度以及其倒数在可以直接使用重心插值
				float z0 = (vs[0]->pos_.z_ + 1.0f) / 2.0f;
				float z1 = (vs[1]->pos_.z_ + 1.0f) / 2.0f;
				float z2 = (vs[2]->pos_.z_ + 1.0f) / 2.0f;
				float depth = a * z0 + b * z1 + c * z2;
				float rhw = vs[0]->rhw_ * a + vs[1]->rhw_ * b + vs[2]->rhw_ * c;

				//depth_buffer数组下标和x,y相反
				auto depth_buffer = m_temp_buffer[theard_num]->get_depth_attachment();
				if (depth > depth_buffer->get_depth(x, y))continue;
				depth_buffer->set_depth(x, y, depth);
				float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

				//透视正确插值
				float c0 = vs[0]->rhw_ * a * w;
				float c1 = vs[1]->rhw_ * b * w;
				float c2 = vs[2]->rhw_ * c * w;


				constexpr size_t count = SHADER::get_context_count();
				float* input=shader.get_output_context(theard_num);
				for (size_t i = 0; i < count; i++) input[i] = c0 * i0[i] + c1 * i1[i] + c2 * i2[i];
				TinyMath::Vec4f color = shader.FragmentShader(input);
				m_temp_buffer[theard_num]->get_attachment(0)->set_pixel_no_cache(x, y, TransformToColor(color));

			}
		}
	

		/*if (m_render_flag & DRAW_LINE)
		{
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[2]->coords_.x_, vs[2]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[2]->coords_.x_, vs[2]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
		}*/

	}


	template<typename VAO, typename SHADER>
	static inline void Submit(const VAO& vao, SHADER& shader)
	{
		const auto& vs = vao.get_vertices();
		const auto& indices = vao.get_indices();
		if (!vs.size() || !indices.size())return;
		int offset = indices.size() /12; 
		assert(offset >= 1);
		auto fun = [&](int left, int right, const auto& ind,int thread_num)
		{
			for (int i = left; i < right; i += 3)
			{
				uint32_t is[3] = { (uint32_t)ind[i],(uint32_t)ind[i + 1],(uint32_t)ind[i + 2] };
				DrawTriangle(vao, shader, is, thread_num);
			}
		};
		std::future<void> res[4];
		res[0]=m_thread_pool.PushTask(fun, 0, offset * 3, indices, 0);
		res[1]=m_thread_pool.PushTask(fun, offset * 3, offset * 6, indices, 1);
		res[2]=m_thread_pool.PushTask(fun, 0 * 6, offset * 9, indices, 2);
		res[3]=m_thread_pool.PushTask(fun, offset * 9, indices.size(), indices, 3);

		for (int i = 0; i < 4; i++)
		{
			res[i].wait();
		}
	}
	
private:
	static Vertex m_vertex[4][3];



	//Buffer
	static DepthAttachment* m_temp_buffer[4];
	static FragmentShader[]
	static FrameBuffer* m_buffer;
	static FrameBuffer* m_default_buffer;

	
	static Color m_color_fg;
	static Color m_color_bg;
	
	static int m_width;
	static int m_height;
	static int m_render_flag;
	static ThreadPool m_thread_pool;
};