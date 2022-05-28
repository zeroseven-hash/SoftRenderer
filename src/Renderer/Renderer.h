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
#include"RendererContext.h"
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
		for (int i = 1; i < CORE_NUM - 1; i++)
		{
			if (m_temp_depth_buffers[i]) delete m_temp_depth_buffers[i];
		}
		if (m_default_buffer) delete m_default_buffer;
		m_thread_pool.Stop();
	}
	template<typename VAO, typename SHADER>
	static void DrawTriangle(const VAO& vao, SHADER& shader, uint32_t* indices,uint32_t trangle_id)
	{
		int min_x = 0, max_x = 0;
		int min_y = 0, max_y = 0;
		
		using ShaderContext = decltype(shader.get_context_type());
		
		
		for (int k = 0;k < 3;k++)
		{
			auto& v = m_triangle_context[trangle_id].vs_[k];
			v.context_ = &m_input_shadercontext[trangle_id * 3*16 + k*16];
			
			v.pos_ = shader.VertexShader(vao, indices[k],*(ShaderContext*)v.context_);

			//�ü� pos(nx,ny,n^2,z);x y should between [-1,1],n could be near or far

			//w_>0;
			
			//if (TinyMath::IsEqual(v.pos_.w_,0.0f,FLT_EPSILON))   return;

			if (v.pos_.x_<-v.pos_.w_ || v.pos_.x_>v.pos_.w_)  return;
			if (v.pos_.y_<-v.pos_.w_ || v.pos_.y_>v.pos_.w_)  return;
			if (v.pos_.z_<-v.pos_.w_ || v.pos_.z_>v.pos_.w_)  return;


			v.rhw_ = 1.0f / v.pos_.w_;
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

	
		////�����߿�
		//if (m_render_flag&DRAW_LINE)
		//{
		//	m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		//	m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_color_fg);
		//	m_canvas->DrawLine(m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		//}
		if (!(m_render_flag&DRAW_PIXEL)) return;

		TinyMath::Vec4f v01 = m_triangle_context[trangle_id].vs_[1].pos_ - m_triangle_context[trangle_id].vs_[0].pos_;
		TinyMath::Vec4f v02 = m_triangle_context[trangle_id].vs_[2].pos_ - m_triangle_context[trangle_id].vs_[0].pos_;
		TinyMath::Vec4f normal = v01.Cross(v02);
		m_triangle_context[trangle_id].ordered_vs_[0] = &m_triangle_context[trangle_id].vs_[0];
		m_triangle_context[trangle_id].ordered_vs_[1] = &m_triangle_context[trangle_id].vs_[1];
		m_triangle_context[trangle_id].ordered_vs_[2] = &m_triangle_context[trangle_id].vs_[2];
		
		
		//back face cull
		if (m_render_flag & FACE_CULL)
		{
			if (normal.z_ <= 0.0) return;
		}
		else
		{
			if (normal.z_ < 0.0f)
				std::swap(m_triangle_context[trangle_id].ordered_vs_[1], m_triangle_context[trangle_id].ordered_vs_[2]);
			else if (normal.z_ == 0.0f)
				return;
		}

		TinyMath::Vec2i p[3] = { m_triangle_context[trangle_id].ordered_vs_[0]->coords_,m_triangle_context[trangle_id].ordered_vs_[1]->coords_ ,m_triangle_context[trangle_id].ordered_vs_[2]->coords_ };

		// ���������Ϊ����˳�
		int si = std::abs((p[1] - p[0]).Cross(p[2] - p[0]));
		if (si <= 0) return;

		m_triangle_context[trangle_id].top_left01_ = IsTopLeft(p[0], p[1]);
		m_triangle_context[trangle_id].top_left12_ = IsTopLeft(p[1], p[2]);
		m_triangle_context[trangle_id].top_left20_ = IsTopLeft(p[2], p[0]);
		m_triangle_context[trangle_id].tag_ = true;
		m_triangle_context[trangle_id].min_x_ = min_x;
		m_triangle_context[trangle_id].max_x_ = max_x;
		m_triangle_context[trangle_id].min_y_ = min_y;
		m_triangle_context[trangle_id].max_y_ = max_y;
		
		
#if 0
		float* i0 = (float*)(ShaderContext*)(&vs[0]->context_);
		float* i1 = (float*)(ShaderContext*)(&vs[1]->context_);
		float* i2 = (float*)(ShaderContext*)(&vs[2]->context_);

//#pragma omp parallel for schdule(static,200)
		for (int y = min_y; y <= max_y; y++)
		{
			for (int x = min_x; x <= max_x; x++)
			{
				// Edge Equation
				// ʹ���������⸡����ͬʱ��Ϊ������ϵ�����Է���ȡ��
				int E01 = -(x - p[0].x_) * (p[1].y_ - p[0].y_) + (y - p[0].y_) * (p[1].x_ - p[0].x_);
				int E12 = -(x - p[1].x_) * (p[2].y_ - p[1].y_) + (y - p[1].y_) * (p[2].x_ - p[1].x_);
				int E20 = -(x - p[2].x_) * (p[0].y_ - p[2].y_) + (y - p[2].y_) * (p[0].x_ - p[2].x_);


				// ��������ϱߣ��� E >= 0 �жϺϷ���������±߾��� E > 0 �жϺϷ�
				// ����ͨ������һ����� 1 ������ < 0 �� <= 0 ��һ��ʽ�ӱ��
				if (E01 < (TopLeft01 ? 0 : 1)) continue;   // �ڵ�һ���ߺ���
				if (E12 < (TopLeft12 ? 0 : 1)) continue;   // �ڵڶ����ߺ���
				if (E20 < (TopLeft20 ? 0 : 1)) continue;   // �ڵ������ߺ���


				// �����˵㵽��ǰ���ʸ��
				TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };
				TinyMath::Vec2f s0 = vs[0]->center_ - pixel;
				TinyMath::Vec2f s1 = vs[1]->center_ - pixel;
				TinyMath::Vec2f s2 = vs[2]->center_ - pixel;

				// ��������ϵ�������ڲ������������ a / b / c
				float a = std::abs(s1.Cross(s2));    // �������� Px-P1-P2 ���
				float b = std::abs(s2.Cross(s0));    // �������� Px-P2-P0 ���
				float c = std::abs(s0.Cross(s1));    // �������� Px-P0-P1 ���
				float s = a + b + c;                 // �������� P0-P1-P2 ���

				if (s == 0.0f) continue;

				// Barycentric coordinates interpolation
				a /= s;
				b /= s;
				c /= s;

				//����Լ��䵹���ڿ���ֱ��ʹ�����Ĳ�ֵ
				float z0 = (vs[0]->pos_.z_ + 1.0f) / 2.0f;
				float z1 = (vs[1]->pos_.z_ + 1.0f) / 2.0f;
				float z2 = (vs[2]->pos_.z_ + 1.0f) / 2.0f;
				float depth = a * z0 + b * z1 + c * z2;
				float rhw = vs[0]->rhw_ * a + vs[1]->rhw_ * b + vs[2]->rhw_ * c;

				//depth_buffer�����±��x,y�෴
				if (depth > m_temp_depth_buffers[theard_num]->get_depth(x, y))continue;
				m_temp_depth_buffers[theard_num]->set_depth(x, y, depth);
				float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

				//͸����ȷ��ֵ
				float c0 = vs[0]->rhw_ * a * w;
				float c1 = vs[1]->rhw_ * b * w;
				float c2 = vs[2]->rhw_ * c * w;


				static ThreadDataWrapper<ShaderContext> output[CORE_NUM];
				float* temp = (float*)(ShaderContext*)(&output[theard_num].data_);
				constexpr int count = sizeof(ShaderContext) / sizeof(float);
				
				for (size_t i = 0; i < count; i++) temp[i] = c0 * i0[i] + c1 * i1[i] + c2 * i2[i];
				TinyMath::Vec4f color = shader.FragmentShader(output[theard_num].data_);

				m_default_buffer->get_attachment(0)->set_pixel(x, y, TransformToColor(color));
			}
		}
	
#endif
		/*if (m_render_flag & DRAW_LINE)
		{
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[2]->coords_.x_, vs[2]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[2]->coords_.x_, vs[2]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
		}*/

	}

	static void Fun(int min_y, int max_y);
	template<typename VAO, typename SHADER>
	static inline void Submit(const VAO& vao, SHADER& shader)
	{
		const auto& vs = vao.get_vertices();
		const auto& indices = vao.get_indices();
		size_t triangle_cout = indices.size() / 3;
		if (m_triangle_context.size() < triangle_cout)
		{
			m_triangle_context.resize(triangle_cout*1.5);
			delete[] m_input_shadercontext;
			m_input_shadercontext = new float[triangle_cout*1.5 * 16 * 3];
		}
		

		if (!vs.size() || !indices.size())return;
		int offset = indices.size() / 12;
		auto fun = [&](int left, int right, const auto& ind)
		{
			int offset = left / 3;
			int count = 0;
			for (int i = left; i < right; i += 3)
			{
				uint32_t is[3] = { (uint32_t)ind[i],(uint32_t)ind[i + 1],(uint32_t)ind[i + 2] };
				DrawTriangle(vao, shader, is, offset + count);
				count++;
			}
		};

		//fun(0, indices.size(), indices);
		std::future<void> res[4];
		res[0]=m_thread_pool.PushTask(fun, 0, offset * 3, indices);
		res[1]=m_thread_pool.PushTask(fun, offset * 3, offset * 6, indices);
		res[2]=m_thread_pool.PushTask(fun, offset * 6, offset * 9, indices);
		res[3]=m_thread_pool.PushTask(fun, offset * 9, indices.size(), indices);

		for (int i = 0; i < 4; i++)
		{
			res[i].wait();
		}

		using ShaderContext = decltype(shader.get_context_type());
		constexpr int count = sizeof(ShaderContext) / sizeof(float);
		static ShaderContext input;
		float* temp = (float*)&input;

		for (int i = 0; i < triangle_cout; i++)
		{
			
			if (!m_triangle_context[i].tag_) continue;
			m_triangle_context[i].tag_ = false;
			int min_x = m_triangle_context[i].min_x_;
			int max_x = m_triangle_context[i].max_x_;
			int min_y = m_triangle_context[i].min_y_;
			int max_y = m_triangle_context[i].max_y_;

			float* i0 = m_triangle_context[i].ordered_vs_[0]->context_;
			float* i1 = m_triangle_context[i].ordered_vs_[1]->context_;
			float* i2 = m_triangle_context[i].ordered_vs_[2]->context_;
			for (int y = min_y; y <= max_y; y++)
			{
				for (int x = min_x; x <= max_x; x++)
				{
					auto& tri_context = m_triangle_context[i];
					auto& p0 = tri_context.ordered_vs_[0]->coords_;
					auto& p1 = tri_context.ordered_vs_[1]->coords_;
					auto& p2 = tri_context.ordered_vs_[2]->coords_;

					int E01 = -(x - p0.x_) * (p1.y_ - p0.y_) + (y - p0.y_) * (p1.x_ - p0.x_);
					int E12 = -(x - p1.x_) * (p2.y_ - p1.y_) + (y - p1.y_) * (p2.x_ - p1.x_);
					int E20 = -(x - p2.x_) * (p0.y_ - p2.y_) + (y - p2.y_) * (p0.x_ - p2.x_);


					// ��������ϱߣ��� E >= 0 �жϺϷ���������±߾��� E > 0 �жϺϷ�
					// ����ͨ������һ����� 1 ������ < 0 �� <= 0 ��һ��ʽ�ӱ��
					if (E01 < (tri_context.top_left01_ ? 0 : 1)) continue;   // �ڵ�һ���ߺ���
					if (E12 < (tri_context.top_left12_ ? 0 : 1)) continue;   // �ڵڶ����ߺ���
					if (E20 < (tri_context.top_left20_ ? 0 : 1)) continue;   // �ڵ������ߺ���


					// �����˵㵽��ǰ���ʸ��
					TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };
					TinyMath::Vec2f s0 = tri_context.ordered_vs_[0]->center_ - pixel;
					TinyMath::Vec2f s1 = tri_context.ordered_vs_[1]->center_ - pixel;
					TinyMath::Vec2f s2 = tri_context.ordered_vs_[2]->center_ - pixel;

					// ��������ϵ�������ڲ������������ a / b / c
					float a = std::abs(s1.Cross(s2));    // �������� Px-P1-P2 ���
					float b = std::abs(s2.Cross(s0));    // �������� Px-P2-P0 ���
					float c = std::abs(s0.Cross(s1));    // �������� Px-P0-P1 ���
					float s = a + b + c;                 // �������� P0-P1-P2 ���

					if (s == 0.0f) continue;

					// Barycentric coordinates interpolation
					a /= s;
					b /= s;
					c /= s;

					//����Լ��䵹���ڿ���ֱ��ʹ�����Ĳ�ֵ
					float z0 = (tri_context.ordered_vs_[0]->pos_.z_ + 1.0f) / 2.0f;
					float z1 = (tri_context.ordered_vs_[1]->pos_.z_ + 1.0f) / 2.0f;
					float z2 = (tri_context.ordered_vs_[2]->pos_.z_ + 1.0f) / 2.0f;
					float depth = a * z0 + b * z1 + c * z2;
					float rhw = tri_context.ordered_vs_[0]->rhw_ * a + tri_context.ordered_vs_[1]->rhw_ * b + tri_context.ordered_vs_[2]->rhw_ * c;

					////depth_buffer�����±��x,y�෴
					auto depth_buffer = m_buffer->get_depth_attachment();
					if (depth > depth_buffer->get_depth(x, y))continue;
					depth_buffer->set_depth(x, y, depth);
					float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

					//͸����ȷ��ֵ
					float c0 = tri_context.ordered_vs_[0]->rhw_ * a * w;
					float c1 = tri_context.ordered_vs_[1]->rhw_ * b * w;
					float c2 = tri_context.ordered_vs_[2]->rhw_ * c * w;

					
					
					for (size_t i = 0; i < count; i++) temp[i] = c0 * i0[i] + c1 * i1[i] + c2 * i2[i];
					TinyMath::Vec4f color = shader.FragmentShader(*(ShaderContext*)temp);
					m_buffer->get_attachment(0)->set_pixel(x, y, TinyMath::TransformToColor(color));
				}
			}

			
			
		}
	}
	
private:



	//Buffer
	static DepthAttachment* m_temp_depth_buffers[CORE_NUM];
	static FrameBuffer* m_buffer;
	static FrameBuffer* m_default_buffer;

	

	static std::vector<std::vector<int>> lines;
	static Color m_color_fg;
	static Color m_color_bg;
	
	static int m_width;
	static int m_height;
	static int m_render_flag;
	static ThreadPool m_thread_pool;

	static std::vector<TaskInfo> temp_buffer;
	static float* m_input_shadercontext;
	static std::vector<TriangleContext> m_triangle_context;
};