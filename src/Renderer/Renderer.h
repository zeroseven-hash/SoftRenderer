#pragma once
#include<stdint.h>
#include<memory>
#include<map>
#include<cstring>
#include<iostream>


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
	static void Clear(BufferFlag flag,const Color& color=Color(0x00,0x00,0x00,0x00),float depth=1.0f);
	static void Bind(FrameBuffer* fb);
	static void UnBind();
	static void SetViewPort(uint32_t width, uint32_t height);
	static void LineColor(const Color& color) { Renderer::GetRenderContext().color_fg_ = color; }
	static void SetState(BufferFlag flag) { Renderer::GetRenderContext().renderer_flag = flag; }
	static void FlushFrame();
	static void FlushFrame(FrameBuffer* fb, int attachment);
	static void Destory(){}



	

	
	template<typename VAO, typename SHADER>
	static inline void Submit(const VAO& vao, SHADER& shader)
	{
		
		constexpr int count = sizeof(SHADER::ContextType) / sizeof(float);

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
				context.input_shadercontexts_.Resize(len, MAXVERTICES * count);
			}

			if (count * MAXVERTICES > context.input_shadercontexts_.count_)
			{
				context.input_shadercontexts_.Resize(triangle_cout, MAXVERTICES * count);
			}
		}
		
		
		if (!vs.size() || !indices.size())return;
		auto fun = [&](int left, int right, const auto& ind)
		{
			for (int i = left; i < right; i++)
			{
				int index = i * 3;
				uint32_t is[3] = { (uint32_t)ind[index],(uint32_t)ind[index + 1],(uint32_t)ind[index + 2] };
				SetupTriangle(vao, shader, is, i );
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
			
			if (!context.triangles_[i].tag_) continue;
			DrawPixel(vao, shader, context.triangles_[i]);
			context.triangles_[i].tag_ = false;
			context.triangles_[i].swap_ = false;
		}
	}
	
	static RendererContext& GetRenderContext()
	{
		return Get().m_context;
	}
private:
	Renderer() = default;
	static Renderer& Get();

	static void ClipTrangle(TriangleContext& trangle,uint32_t trangle_id,int varying_size);

	template<typename VAO,typename SHADER>
	static void SetupTriangle(const VAO& vao, SHADER& shader, uint32_t* indices, uint32_t trangle_id)
	{
		auto& context = Renderer::GetRenderContext();
		int width = context.width_;
		int height = context.height_;
		for (int k = 0; k < 3; k++)
		{
			auto& v = context.triangles_[trangle_id].input_vs_[k];
			int count = context.input_shadercontexts_.count_;
			context.triangles_[trangle_id].vertices_nums = 3;
			v.context_ = &context.input_shadercontexts_[trangle_id * count+ k * count/MAXVERTICES];

			v.pos_ = shader.VertexShader(vao, indices[k], *(SHADER::ContextType*)v.context_);
		}


		constexpr int count = sizeof(SHADER::ContextType)/sizeof(float);
		//clipping
		ClipTrangle(context.triangles_[trangle_id],trangle_id,count);

		int v_cnt = (int)context.triangles_[trangle_id].vertices_nums;
		if (v_cnt < 3) return;
		int tri_cnt = v_cnt - 2;
		for (int i = 0; i < v_cnt; i++)
		{
			auto& v = context.triangles_[trangle_id].output_vs_[i];
			//1/w'
			v.rhw_ = 1.0f / v.pos_.w_;
			v.pos_ *= v.rhw_;

			
			//x:[-1,1] to [0,width],y:[-1,1] to [0,height];
			//	0.5*width       0	      width*0.5		    x     (x+1.0f)*0.5*width
			//	  0	       height*0.5    height*0.5    *    y  = (y+1.0f)*0.5*height
			//	  0             0            0	        	1 				1
			v.ndc_coords_.x_ = (v.pos_.x_ + 1.0f) *(float) (width-1)* 0.5f;
			v.ndc_coords_.y_ = (v.pos_.y_ + 1.0f) * (float)(height-1)* 0.5f;
			v.ndc_coords_.z_ = (v.pos_.z_ + 1.0f) * 0.5f;

			v.inter_coords.x_ = (int)v.ndc_coords_.x_;
			v.inter_coords.y_ = (int)v.ndc_coords_.y_;
		}


		

		auto& triangles = context.triangles_[trangle_id];
		for (int i = 0; i < tri_cnt; i++)
		{
			auto& v0 = triangles.output_vs_[0];
			auto& v1 = triangles.output_vs_[i + 1];
			auto& v2 = triangles.output_vs_[i + 2];


			TinyMath::Vec3f v01 = v1.ndc_coords_ - v0.ndc_coords_;
			TinyMath::Vec3f v02 = v2.ndc_coords_ - v1.ndc_coords_;
			TinyMath::Vec3f normal = v01.Cross(v02);
					
			
			//back face cull
			if (context.renderer_flag & FACE_CULL)
			{
				if (normal.z_ <= 0.0) return;
			}
			else
			{
				if (normal.z_ < 0.0f)
				{
					triangles.swap_ = true;
					break;
				}
			}
		}
		if(tri_cnt>0) context.triangles_[trangle_id].tag_ = true;
	}
	
	template<typename VAO,typename SHADER>
	static void DrawPixel(const VAO& vao, SHADER& shader,TriangleContext& tri_context)
	{
		constexpr int count = sizeof(SHADER::ContextType) / sizeof(float);
		
		uint32_t tri_cnt = tri_context.vertices_nums - 2;
		auto& context = Renderer::GetRenderContext();
		int width = (int)context.width_;
		int height = (int)context.height_;


		for (uint32_t i = 0; i < tri_cnt; i++)
		{
			Vertex* v0 = &tri_context.output_vs_[0];
			Vertex* v1 = &tri_context.output_vs_[i + 1];
			Vertex* v2 = &tri_context.output_vs_[i + 2];
			if (tri_context.swap_) std::swap(v1, v2);
			if (context.renderer_flag & DRAW_LINE)
			{
				DrawLine(v0->inter_coords.x_, v0->inter_coords.y_, v1->inter_coords.x_, v1->inter_coords.y_, context.color_fg_);
				DrawLine(v0->inter_coords.x_, v0->inter_coords.y_, v2->inter_coords.x_, v2->inter_coords.y_, context.color_fg_);
				DrawLine(v2->inter_coords.x_, v2->inter_coords.y_, v1->inter_coords.x_, v1->inter_coords.y_, context.color_fg_);
			}

			if (!(context.renderer_flag & DRAW_PIXEL)) return;

			
			int max_x = TinyMath::Between(0, width - 1, std::max({ v0->inter_coords.x_,v1->inter_coords.x_,v2->inter_coords.x_ }));
			int min_x = TinyMath::Between(0, width - 1, std::min({ v0->inter_coords.x_,v1->inter_coords.x_,v2->inter_coords.x_ }));
			int min_y = TinyMath::Between(0, height - 1, std::min({ v0->inter_coords.y_,v1->inter_coords.y_,v2->inter_coords.y_ }));
			int max_y = TinyMath::Between(0, height - 1, std::max({ v0->inter_coords.y_,v1->inter_coords.y_,v2->inter_coords.y_ }));

			
			float* i0 = v0->context_;
			float* i1 = v1->context_;
			float* i2 = v2->context_;



			//for calculate ddx and ddy,use quad the calculte pixel;
			static struct{
				int x_;
				int y_;
				bool is_in_ = false;
				SHADER::ContextType input_;
			} quad[4];	//

			const int dir[4][2] = { {0,0},{1,0},{0,1},{1,1} };
			int offset_x = max_x - min_x + 1;
			int offset_y = max_y - min_y + 1;
			offset_x = (offset_x & 1) ? offset_x + 1 : offset_x;
			offset_y = (offset_y & 1) ? offset_y + 1 : offset_y;
			int block_x = offset_x / 2;
			int block_y = offset_y / 2;
			for (int by = 0; by < block_y; by++)
			{
				for (int bx = 0; bx < block_x; bx++)
				{

					//interpolate attributes.
					for (int i = 0; i < 4; i++)
					{
						quad[i].x_ = min_x + 2 * bx + dir[i][0];
						quad[i].y_ = min_y + 2 * by + dir[i][1];

						if (quad[i].x_ >= width - 1 || quad[i].y_ >= width - 1) { quad[i].is_in_ = false; continue;}
						// 三个端点到当前点的矢量
						TinyMath::Vec2f pixel = { (float)quad[i].x_ + 0.5f,(float)quad[i].y_ + 0.5f };
						TinyMath::Vec2f s0 = { v0->ndc_coords_.x_ - pixel.x_,v0->ndc_coords_.y_ - pixel.y_ };
						TinyMath::Vec2f s1 = { v1->ndc_coords_.x_ - pixel.x_,v1->ndc_coords_.y_ - pixel.y_ };
						TinyMath::Vec2f s2 = { v2->ndc_coords_.x_ - pixel.x_,v2->ndc_coords_.y_ - pixel.y_ };

						// 重心坐标系：计算内部子三角形面积 a / b / c
						float a = s1.Cross(s2);	   // 子三角形 Px-P1-P2 面积
						float b = s2.Cross(s0);    // 子三角形 Px-P2-P0 面积
						float c = s0.Cross(s1);    // 子三角形 Px-P0-P1 面积
						float s = a + b + c;       // 大三角形 P0-P1-P2 面积

						if (a < 0.0f || b < 0.0f || c < 0.0f || s == 0.0f) { quad[i].is_in_ = false; continue; }

						// Barycentric coordinates interpolation
						a /= s;
						b /= s;
						c /= s;

						//深度以及其倒数在可以直接使用重心插值
						float z0 = v0->ndc_coords_.z_;
						float z1 = v1->ndc_coords_.z_;
						float z2 = v2->ndc_coords_.z_;
						float depth = a * z0 + b * z1 + c * z2;
						float rhw = v0->rhw_ * a + v1->rhw_ * b + v2->rhw_ * c;

						////depth_buffer数组下标和x,y相反
						auto depth_buffer = m_buffer->get_depth_attachment();
						if (depth > depth_buffer->get_depth(quad[i].x_, quad[i].y_)) { quad[i].is_in_ = false; continue; }
						depth_buffer->set_depth(quad[i].x_, quad[i].y_, depth);
						float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

						//透视正确插值
						float c0 = v0->rhw_ * a * w;
						float c1 = v1->rhw_ * b * w;
						float c2 = v2->rhw_ * c * w;

						float* temp = (float*)&quad[i].input_;
						for (int i = 0; i < count; i++) temp[i] = c0 * i0[i] + c1 * i1[i] + c2 * i2[i];
						quad[i].is_in_ = true;
					}

					static SHADER::ContextType delta[2];
					//calcualte ddx and ddy
					float* delta_x = (float*)&delta[0];
					float* delta_y = (float*)&delta[1];
					if (quad[0].is_in_ && quad[1].is_in_)
					{
						for (int i = 0; i < count; i++)
						{
							delta_x[i] = ((float*)(&quad[1].input_))[i] - ((float*)(&quad[0].input_))[i];
						}
					}
					else
						delta_x[i] = 0.0f;
					
					if (quad[0].is_in_ && quad[2].is_in_)
					{
						for (int i = 0; i < count; i++)
						{
							delta_y[i] = ((float*)(&quad[2].input_))[i] - ((float*)(&quad[0].input_))[i];
						}
					}
					else
						delta_y[i] = 0.0f;
					
					for (int i = 0; i < 4; i++)
					{
						if (quad[i].is_in_)
						{
							TinyMath::Vec4f color = shader.FragmentShader(&quad[i].input_,delta);
							m_buffer->get_attachment(0)->set_pixel(quad[i].x_, quad[i].y_, TinyMath::TransformToColor(color));
						}
					}
				}
			}



		


		}
	
	}

	static void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
private:

	RendererContext m_context;

	//Buffer
	static FrameBuffer* m_buffer;	

};