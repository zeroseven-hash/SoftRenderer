#pragma once
#include<stdint.h>
#include<memory>
#include<map>
#include<cstring>


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
	inline ~Renderer() {  }
public:




	static void Init(uint32_t width,uint32_t height);
	static void Clear(BufferFlag flag,const Color& color=Color(0x00,0x00,0x00,0x00d),float depth=1.0f);
	static void Bind(FrameBuffer* fb);
	static void UnBind();
	static void SetViewPort(uint32_t width, uint32_t height);
	static void LineColor(const Color& color) { Renderer::GetRenderContext().color_fg_ = color; }
	static void SetState(RenderFlag_ flag) { Renderer::GetRenderContext().renderer_flag = flag; }
	static void FlushFrame();
	static void FlushFrame(FrameBuffer* fb, int attachment);
	static void Destory(){}



	

	
	template<typename VAO, typename SHADER>
	static inline void Submit(const VAO& vao, SHADER& shader)
	{
		using ShaderContext = decltype(shader.get_context_type());
		constexpr int count = sizeof(ShaderContext) / sizeof(float);

		const auto& vs = vao.get_vertices();
		const auto& indices = vao.get_indices();
		uint32_t triangle_cout = (uint32_t)indices.size() / 3;


		auto& context = Renderer::GetRenderContext();

		{
			//resize cache;
			if (context.triangles_.length_ < triangle_cout)
			{
				int len = int(triangle_cout * 1.5f);
				context.triangles_.Resize(len, 1);
				context.input_shadercontexts_.Resize(len, 3 * count);
			}

			if (count * 3 > context.input_shadercontexts_.count_)
			{
				context.input_shadercontexts_.Resize(triangle_cout, 3 * count);
			}
		}
		
		
		if (!vs.size() || !indices.size())return;
		auto fun = [&](int left, int right, const auto& ind)
		{
			for (int i = left; i < right; i++)
			{
				int index = i * 3;
				uint32_t is[3] = { (uint32_t)ind[index],(uint32_t)ind[index + 1],(uint32_t)ind[index + 2] };
				DrawTriangle(vao, shader, is, i );
			}
		};

		//distribute task;
		{
			int thread_num = ThreadPool::GetThreadNums();
			int primitive_count = triangle_cout;
			int offset = primitive_count / thread_num ;
			

			
			std::vector<std::future<void>> res(thread_num);
			int left = 0;
			for (int i = 0; i < thread_num; i++)
			{
				int right = i==(thread_num-1)? primitive_count :left + offset;
				res[i] = ThreadPool::PushTask(fun, left, right, indices);
				left = right;
			}

			for (int i = 0; i < thread_num; i++)
			{
				res[i].wait();
			}
		}
		

		
		//Run depth and fragmentshader 
		for (uint32_t i = 0; i < triangle_cout; i++)
		{
			
			if (!context.triangles_.data_[i].tag_) continue;
			context.triangles_.data_[i].tag_ = false;
			DrawPixel(vao, shader, context.triangles_.data_[i]);
		}
	}
	
	static RendererContext& GetRenderContext()
	{
		return Get().m_context;
	}
private:
	Renderer() = default;
	static Renderer& Get();


	template<typename VAO,typename SHADER>
	static void DrawTriangle(const VAO& vao, SHADER& shader, uint32_t* indices, uint32_t trangle_id)
	{
		int min_x, max_x;
		int min_y, max_y;

		using ShaderContext = decltype(shader.get_context_type());

		auto& context = Renderer::GetRenderContext();
		int width = context.width_;
		int height = context.height_;
		for (int k = 0; k < 3; k++)
		{
			auto& v = context.triangles_.data_[trangle_id].vs_[k];
			int count = context.input_shadercontexts_.count_;
			v.context_ = &context.input_shadercontexts_.data_[trangle_id * count+ k * count/3];

			v.pos_ = shader.VertexShader(vao, indices[k], *(ShaderContext*)v.context_);

			//裁剪 pos(nx,ny,n^2,z);x y should between [-1,1],n could be near or far

			//w_>0;
			if (TinyMath::IsEqual(v.pos_.w_, 0.0f, FLT_EPSILON))   return;

			if (v.pos_.x_<-v.pos_.w_ || v.pos_.x_>v.pos_.w_)  return;
			if (v.pos_.y_<-v.pos_.w_ || v.pos_.y_>v.pos_.w_)  return;
			if (v.pos_.z_<-v.pos_.w_ || v.pos_.z_>v.pos_.w_)  return;


			v.rhw_ = 1.0f / v.pos_.w_;
			v.pos_ *= v.rhw_;

			
			//x:[-1,1] to [0,width-1],y:[-1,1] to [0,height-1];
			//	0.5*width       0	      width*0.5		    x     (x+1.0f)*0.5*width
			//	  0	       height*0.5    height*0.5    *    y  = (y+1.0f)*0.5*height
			//	  0             0            0	        	1 				1
			v.center_.x_ = (v.pos_.x_ + 1.0f) *(float) width* 0.5f;
			v.center_.y_ = (v.pos_.y_ + 1.0f) * (float)height* 0.5f;


			v.coords_.x_ = (int)(v.center_.x_ + 0.5f);
			v.coords_.y_ = (int)(v.center_.y_ + 0.5f);

			if (k == 0)
			{
				min_x = max_x = TinyMath::Between(0, width- 1, v.coords_.x_);
				min_y = max_y = TinyMath::Between(0, height- 1, v.coords_.y_);
			}
			else
			{
				min_x = TinyMath::Between(0, width- 1, std::min(min_x, v.coords_.x_));
				max_x = TinyMath::Between(0, width- 1, std::max(max_x, v.coords_.x_));
				min_y = TinyMath::Between(0, height - 1, std::min(min_y, v.coords_.y_));
				max_y = TinyMath::Between(0, height - 1, std::max(max_y, v.coords_.y_));
			}
		}


		////绘制线框
		//if (m_render_flag&DRAW_LINE)
		//{
		//	m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		//	m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_color_fg);
		//	m_canvas->DrawLine(m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_, m_color_fg);
		//}
		if (!(context.renderer_flag & DRAW_PIXEL)) return;

		TinyMath::Vec4f v01 = context.triangles_.data_[trangle_id].vs_[1].pos_ - context.triangles_.data_[trangle_id].vs_[0].pos_;
		TinyMath::Vec4f v02 = context.triangles_.data_[trangle_id].vs_[2].pos_ - context.triangles_.data_[trangle_id].vs_[0].pos_;
		TinyMath::Vec4f normal = v01.Cross(v02);
		context.triangles_.data_[trangle_id].ordered_vs_[0] = &context.triangles_.data_[trangle_id].vs_[0];
		context.triangles_.data_[trangle_id].ordered_vs_[1] = &context.triangles_.data_[trangle_id].vs_[1];
		context.triangles_.data_[trangle_id].ordered_vs_[2] = &context.triangles_.data_[trangle_id].vs_[2];


		//back face cull
		if (context.renderer_flag & FACE_CULL)
		{
			if (normal.z_ <= 0.0) return;
		}
		else
		{
			if (normal.z_ < 0.0f)
				std::swap(context.triangles_.data_[trangle_id].ordered_vs_[1], context.triangles_.data_[trangle_id].ordered_vs_[2]);
			else if (normal.z_ == 0.0f)
				return;
		}

		TinyMath::Vec2i p[3] = { context.triangles_.data_[trangle_id].ordered_vs_[0]->coords_,context.triangles_.data_[trangle_id].ordered_vs_[1]->coords_ ,context.triangles_.data_[trangle_id].ordered_vs_[2]->coords_ };

		// 计算面积，为零就退出
		int si = std::abs((p[1] - p[0]).Cross(p[2] - p[0]));
		if (si <= 0) return;

		context.triangles_.data_[trangle_id].top_left01_ = IsTopLeft(p[0], p[1]);
		context.triangles_.data_[trangle_id].top_left12_ = IsTopLeft(p[1], p[2]);
		context.triangles_.data_[trangle_id].top_left20_ = IsTopLeft(p[2], p[0]);
		context.triangles_.data_[trangle_id].tag_ = true;
		context.triangles_.data_[trangle_id].min_x_ = min_x;
		context.triangles_.data_[trangle_id].max_x_ = max_x;
		context.triangles_.data_[trangle_id].min_y_ = min_y;
		context.triangles_.data_[trangle_id].max_y_ = max_y;

		/*if (m_render_flag & DRAW_LINE)
		{
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[2]->coords_.x_, vs[2]->coords_.y_, m_color_fg);
			m_canvas->DrawLine(vs[2]->coords_.x_, vs[2]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
		}*/
	}
	
	template<typename VAO,typename SHADER>
	static void DrawPixel(const VAO& vao, SHADER& shader,TriangleContext& tri_context)
	{
		using ShaderContext = decltype(shader.get_context_type());
		constexpr int count = sizeof(ShaderContext) / sizeof(float);
		static ShaderContext input;
		float* temp = (float*)&input;


		int min_x = tri_context.min_x_;
		int max_x = tri_context.max_x_;
		int min_y = tri_context.min_y_;
		int max_y = tri_context.max_y_;

		float* i0 = tri_context.ordered_vs_[0]->context_;
		float* i1 = tri_context.ordered_vs_[1]->context_;
		float* i2 = tri_context.ordered_vs_[2]->context_;
		for (int y = min_y; y <= max_y; y++)
		{
			for (int x = min_x; x <= max_x; x++)
			{
				auto& p0 = tri_context.ordered_vs_[0]->coords_;
				auto& p1 = tri_context.ordered_vs_[1]->coords_;
				auto& p2 = tri_context.ordered_vs_[2]->coords_;

				int E01 = -(x - p0.x_) * (p1.y_ - p0.y_) + (y - p0.y_) * (p1.x_ - p0.x_);
				int E12 = -(x - p1.x_) * (p2.y_ - p1.y_) + (y - p1.y_) * (p2.x_ - p1.x_);
				int E20 = -(x - p2.x_) * (p0.y_ - p2.y_) + (y - p2.y_) * (p0.x_ - p2.x_);


				// 如果是左上边，用 E >= 0 判断合法，如果右下边就用 E > 0 判断合法
				// 这里通过引入一个误差 1 ，来将 < 0 和 <= 0 用一个式子表达
				if (E01 < (tri_context.top_left01_ ? 0 : 1)) continue;   // 在第一条边后面
				if (E12 < (tri_context.top_left12_ ? 0 : 1)) continue;   // 在第二条边后面
				if (E20 < (tri_context.top_left20_ ? 0 : 1)) continue;   // 在第三条边后面


				// 三个端点到当前点的矢量
				TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };
				TinyMath::Vec2f s0 = tri_context.ordered_vs_[0]->center_ - pixel;
				TinyMath::Vec2f s1 = tri_context.ordered_vs_[1]->center_ - pixel;
				TinyMath::Vec2f s2 = tri_context.ordered_vs_[2]->center_ - pixel;

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
				float z0 = (tri_context.ordered_vs_[0]->pos_.z_ + 1.0f) / 2.0f;
				float z1 = (tri_context.ordered_vs_[1]->pos_.z_ + 1.0f) / 2.0f;
				float z2 = (tri_context.ordered_vs_[2]->pos_.z_ + 1.0f) / 2.0f;
				float depth = a * z0 + b * z1 + c * z2;
				float rhw = tri_context.ordered_vs_[0]->rhw_ * a + tri_context.ordered_vs_[1]->rhw_ * b + tri_context.ordered_vs_[2]->rhw_ * c;

				////depth_buffer数组下标和x,y相反
				auto depth_buffer = m_buffer->get_depth_attachment();
				if (depth > depth_buffer->get_depth(x, y))continue;
				depth_buffer->set_depth(x, y, depth);
				float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

				//透视正确插值
				float c0 = tri_context.ordered_vs_[0]->rhw_ * a * w;
				float c1 = tri_context.ordered_vs_[1]->rhw_ * b * w;
				float c2 = tri_context.ordered_vs_[2]->rhw_ * c * w;



				for (size_t i = 0; i < count; i++) temp[i] = c0 * i0[i] + c1 * i1[i] + c2 * i2[i];
				TinyMath::Vec4f color = shader.FragmentShader(*(ShaderContext*)temp);
				m_buffer->get_attachment(0)->set_pixel(x, y, TinyMath::TransformToColor(color));
			}
		}
	}

private:

	RendererContext m_context;

	//Buffer
	static FrameBuffer* m_buffer;	

};