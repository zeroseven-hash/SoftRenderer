#include"Renderer.h"
#include"Utils.h"


#include<algorithm>

#include"Timer.h"
#include<glad/glad.h>



FrameBuffer* Renderer::m_buffer;
/*----------------------------------------------------------------------------------------------------*/
//clipping functions
//for more information,see https://zhuanlan.zhihu.com/p/162190576
enum Plane
{
	POSITIVE_W,
	POSITIVE_X,
	NEGATIVE_X,
	POSITIVE_Y,
	NEGATIVE_Y,
	POSITIVE_Z,
	NEGATIVE_Z
};

static bool IsInsidePlane(Plane plane, const TinyMath::Vec4f& pos)
{
	switch (plane) {
	case POSITIVE_W:
		return pos.w_ >= 1e-5f;
	case POSITIVE_X:
		return pos.x_ <= +pos.w_;
	case NEGATIVE_X:
		return pos.x_ >= -pos.w_;
	case POSITIVE_Y:
		return pos.y_ <= +pos.w_;
	case NEGATIVE_Y:
		return pos.y_ >= -pos.w_;
	case POSITIVE_Z:
		return pos.z_ <= +pos.w_;
	case NEGATIVE_Z:
		return pos.z_ >= -pos.w_;
	default: assert(false); return false;
	}
}

static float CalRatio(const TinyMath::Vec4f& prev, const TinyMath::Vec4f& curr, Plane plane)
{
	switch (plane) {
	case POSITIVE_W:
		return (prev.w_ - 1e-5f) / (prev.w_ - curr.w_);
	case POSITIVE_X:
		return (prev.w_ - prev.x_) / ((prev.w_ - prev.x_) - (curr.w_ - curr.x_));
	case NEGATIVE_X:
		return (prev.w_ + prev.x_) / ((prev.w_ + prev.x_) - (curr.w_ + curr.x_));
	case POSITIVE_Y:
		return (prev.w_ - prev.y_) / ((prev.w_ - prev.y_) - (curr.w_ - curr.y_));
	case NEGATIVE_Y:
		return (prev.w_ + prev.y_) / ((prev.w_ + prev.y_) - (curr.w_ + curr.y_));
	case POSITIVE_Z:
		return (prev.w_ - prev.z_) / ((prev.w_ - prev.z_) - (curr.w_ - curr.z_));
	case NEGATIVE_Z:
		return (prev.w_ + prev.z_) / ((prev.w_ + prev.z_) - (curr.w_ + curr.z_));
	default:
		assert(false);return 0.0f;
	}
}

static bool IsVertexVisable(const TinyMath::Vec4f& pos)
{
	return std::abs(pos.x_) <= pos.w_ && std::abs(pos.y_) <= pos.w_ && std::abs(pos.z_) <= pos.w_;
}



static int Clip(
	Vertex	 input_vs[MAXVERTICES],
	Vertex	 output_vs[MAXVERTICES],
	int		 in_vertices_num,
	int		 varying_size,
	Plane	 plane,
	Utils::CirCleQueue<float*,MAXVERTICES>& queue				//to find which context memery could be used;
)
{
	int out_vertices_num = 0;
	assert(in_vertices_num >= 3 && in_vertices_num <= MAXVERTICES);
	for (int i = 0; i < in_vertices_num; i++)
	{
		int prev_index = (i - 1 + in_vertices_num) % in_vertices_num;
		int curr_index = i;

		auto& prev_pos = input_vs[prev_index].pos_;
		auto& curr_pos = input_vs[curr_index].pos_;

		float* prev_context = input_vs[prev_index].context_;
		float* curr_context = input_vs[curr_index].context_;

		bool prev_inside = IsInsidePlane(plane, prev_pos);
		bool curr_insde = IsInsidePlane(plane, curr_pos);

		if (prev_inside != curr_insde)
		{
			auto& dest_pos = output_vs[out_vertices_num].pos_;

			//find fist memory could used
			assert(!queue.Empty());
			float* dest_context = queue.Front();
			queue.Pop();
			output_vs[out_vertices_num].context_ = dest_context;
			//calucate t
			float ratio = CalRatio(prev_pos, curr_pos, plane);

			dest_pos = TinyMath::LinerInterpolation(prev_pos, curr_pos, ratio);
			for (int j = 0; j < varying_size; j++)
			{
				dest_context[j] = TinyMath::LinerInterpolation(prev_context[j], curr_context[j], ratio);
			}
			out_vertices_num++;
		}
		
		if (curr_insde)
		{
			output_vs[out_vertices_num] = input_vs[curr_index];
			out_vertices_num++;
		}
		else
		{
			//may have bugs;
			queue.Push(curr_context);
		}

	}
	assert(out_vertices_num <= MAXVERTICES);
	return out_vertices_num;
}


/*----------------------------------------------------------------------------------------------------*/


void Renderer::Init(uint32_t width, uint32_t height)
{
	RendererContext& context = Renderer::GetRenderContext();
	context.width_ = width;
	context.height_ = height;
	context.renderer_flag = DRAW_PIXEL;

	TextureSpecList spec = { {TextureLayout::LINEAR,TextureFormat::RGBA8,0} };
	context.default_buffer_ = new FrameBuffer(width, height, spec);
	context.input_shadercontexts_.Resize(2000, 3*16);
	context.triangles_.Resize(2000,1);

	
	m_buffer = context.default_buffer_;
	glViewport(0,0,width, height);
}

void Renderer::Clear(BufferFlag flag, const Color& color, float depth)
{
	m_buffer->Clear(color,depth);
	
}

void Renderer::Bind(FrameBuffer* fb)
{
	m_buffer = fb;
}

void Renderer::UnBind()
{
	m_buffer = Renderer::GetRenderContext().default_buffer_;
}

void Renderer::SetViewPort(uint32_t width, uint32_t height)
{
	RendererContext& context = Renderer::GetRenderContext();
	context.width_ = width;
	context.height_ = height;
	context.default_buffer_->Resize(width, height);
	glViewport(0, 0, width, height);
}

void Renderer::FlushFrame()
{	
	RendererContext& context = Renderer::GetRenderContext();
	auto default_buf = context.default_buffer_;
	glDrawPixels(default_buf->get_width(), default_buf->get_height(), GL_RGBA, GL_UNSIGNED_BYTE, m_buffer->get_attachment()->get_bits());
}

void Renderer::FlushFrame(FrameBuffer* fb, int attachment)
{
	auto texture2d = fb->get_attachment(attachment);
	glDrawPixels(fb->get_width(),fb->get_height(), GL_RGBA, GL_UNSIGNED_BYTE, texture2d->get_bits());
}

Renderer& Renderer::Get()
{
	static Renderer s_instance;
	return s_instance;
}

void Renderer::ClipTrangle(TriangleContext& triangle,uint32_t triangle_id, int varying_size)
{
	bool is_v0 = IsVertexVisable(triangle.input_vs_[0].pos_);
	bool is_v1 = IsVertexVisable(triangle.input_vs_[1].pos_);
	bool is_v2 = IsVertexVisable(triangle.input_vs_[2].pos_);

	if (is_v0 && is_v1 && is_v2)
	{
		triangle.vertices_nums = 3;
		for (uint32_t i = 0; i < triangle.vertices_nums; i++)
		{
			triangle.output_vs_[i] = triangle.input_vs_[i];
		}
	}
	else
	{
		//clipping
		Utils::CirCleQueue<float*, MAXVERTICES> q;
		auto& context = Renderer::GetRenderContext();
		uint32_t offset = triangle_id * context.input_shadercontexts_.count_;
		uint32_t count = context.input_shadercontexts_.count_ / MAXVERTICES;
		q.Clear();
		for (int i = 3; i < MAXVERTICES-1; i++)
		{
			q.Push(&context.input_shadercontexts_[offset + i * count]);
		}

		triangle.vertices_nums = Clip(triangle.input_vs_, triangle.output_vs_, triangle.vertices_nums, varying_size, POSITIVE_W, q);
		if (triangle.vertices_nums < 3) return;
		triangle.vertices_nums = Clip(triangle.output_vs_, triangle.input_vs_, triangle.vertices_nums, varying_size, POSITIVE_X, q);
		if (triangle.vertices_nums < 3) return;

		triangle.vertices_nums = Clip(triangle.input_vs_, triangle.output_vs_, triangle.vertices_nums, varying_size, NEGATIVE_X, q);
		if (triangle.vertices_nums < 3) return;

		triangle.vertices_nums = Clip(triangle.output_vs_, triangle.input_vs_, triangle.vertices_nums, varying_size, POSITIVE_Y, q);
		if (triangle.vertices_nums < 3) return;

		triangle.vertices_nums = Clip(triangle.input_vs_, triangle.output_vs_, triangle.vertices_nums, varying_size, NEGATIVE_Y, q);
		if (triangle.vertices_nums < 3) return;

		triangle.vertices_nums = Clip(triangle.output_vs_, triangle.input_vs_, triangle.vertices_nums, varying_size, POSITIVE_Z, q);
		if (triangle.vertices_nums < 3) return;

		triangle.vertices_nums = Clip(triangle.input_vs_, triangle.output_vs_, triangle.vertices_nums, varying_size, NEGATIVE_Z, q);
		if (triangle.vertices_nums < 3) return;

		
	}
	

}

void Renderer::DrawLine(int x1, int y1, int x2, int y2, const Color& color)
{
	//bresenham 画线算法，由于x，y都在第一象限，所以只有两种情况
	int x, y;
	if (x1 == x2 && y1 == y2) {
		m_buffer->get_attachment(0)->set_pixel(x1, y1, color);
		return;
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) m_buffer->get_attachment(0)->set_pixel(x1, y, color);
		m_buffer->get_attachment(0)->set_pixel(x2, y2, color);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) m_buffer->get_attachment(0)->set_pixel(x, y1, color);
		m_buffer->get_attachment(0)->set_pixel(x2, y2, color);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		int rem = 0;
		if (dx >= dy) {	//slope <1;
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x < x2; x++) {
				m_buffer->get_attachment(0)->set_pixel(x, y, color);
				rem += dy;
				if (rem >= dx) { rem -= dx; y += (y2 >= y1) ? 1 : -1; m_buffer->get_attachment(0)->set_pixel(x, y, color); }
			}
			m_buffer->get_attachment(0)->set_pixel(x2, y2, color);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y < y2; y++) {
				m_buffer->get_attachment(0)->set_pixel(x, y, color);
				rem += dx;
				if (rem >= dy) { rem -= dy; x += (x2 >= x1) ? 1 : -1; m_buffer->get_attachment(0)->set_pixel(x, y, color); }
			}
			m_buffer->get_attachment(0)->set_pixel(x2, y2, color);
		}
	}
}

