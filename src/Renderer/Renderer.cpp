#include"Renderer.h"

#include<algorithm>

#include"Timer.h"
#include<glad/glad.h>



DepthAttachment* Renderer::m_temp_depth_buffers[4];
FrameBuffer* Renderer::m_buffer;
FrameBuffer* Renderer::m_default_buffer;
std::vector<std::vector<int>> Renderer::lines;
std::vector<TaskInfo> Renderer::temp_buffer;
float* Renderer::m_input_shadercontext;
std::vector<TriangleContext> Renderer::m_triangle_context;
Color Renderer::m_color_fg;
Color Renderer::m_color_bg;

int Renderer::m_width;
int Renderer::m_height;
int Renderer::m_render_flag;

ThreadPool Renderer::m_thread_pool(4);
//void Canvas::DrawLine(int x1, int y1, int x2, int y2, const Color& color)
//{
//	//bresenham 画线算法，由于x，y都在第一象限，所以只有两种情况
//	int x, y;
//	if (x1 == x2 && y1 == y2) {
//		set_pixel(x1, y1, color);
//		return;
//	}
//	else if (x1 == x2) {
//		int inc = (y1 <= y2) ? 1 : -1;
//		for (y = y1; y != y2; y += inc) set_pixel(x1, y, color);
//		set_pixel(x2, y2, color);
//	}
//	else if (y1 == y2) {
//		int inc = (x1 <= x2) ? 1 : -1;
//		for (x = x1; x != x2; x += inc) set_pixel(x, y1, color);
//		set_pixel(x2, y2, color);
//	}
//	else {
//		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
//		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
//		int rem = 0;
//		if (dx >= dy) {	//slope <1;
//			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
//			for (x = x1, y = y1; x <= x2; x++) {
//				set_pixel(x, y, color);
//				rem += dy;
//				if (rem >= dx) { rem -= dx; y += (y2 >= y1) ? 1 : -1; set_pixel(x, y, color); }
//			}
//			set_pixel(x2, y2, color);
//		}
//		else {
//			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
//			for (x = x1, y = y1; y <= y2; y++) {
//				set_pixel(x, y, color);
//				rem += dx;
//				if (rem >= dy) { rem -= dy; x += (x2 >= x1) ? 1 : -1; set_pixel(x, y, color); }
//			}
//			set_pixel(x2, y2, color);
//		}
//	}
//}

void Renderer::Init(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	TextureSpecList spec = { {TextureLayout::LINEAR,0,0} };

	m_default_buffer = new FrameBuffer(width,height,spec);
	for (int i = 1; i < CORE_NUM; i++) m_temp_depth_buffers[i] = new DepthAttachment(width, height);
	m_temp_depth_buffers[0] = m_default_buffer->get_depth_attachment();

	lines.resize(m_height, std::vector<int>(2, 0));
	temp_buffer.resize(m_width * m_height);
	for (auto& t : temp_buffer)
	{
		t.triangle_ids_.resize(1);
	}

	m_input_shadercontext = new float[2000 * 16*3];
	m_triangle_context.resize(2000);
	m_color_bg = Color(0x00, 0x00, 0x00, 0x00);
	m_color_fg = Color(0xff, 0xff, 0xff, 0xff);
	m_render_flag = DRAW_PIXEL;
	m_buffer = m_default_buffer;
	glViewport(0,0,width, height);
}

void Renderer::Clear(BufferFlag flag, const Color& color, float depth)
{
	m_buffer->Clear(color,depth);
	
}

void Renderer::Bind(FrameBuffer* fb)
{
	m_buffer = fb;
	m_temp_depth_buffers[0] = m_buffer->get_depth_attachment();
}

void Renderer::UnBind()
{
	m_buffer = m_default_buffer;
	m_temp_depth_buffers[0] = m_buffer->get_depth_attachment();
}

void Renderer::SetViewPort(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	for (int i = 1; i < CORE_NUM; i++) m_temp_depth_buffers[i]->Resize(width, height);
	m_default_buffer->Resize(width, height);
	glViewport(0, 0, width, height);
}

void Renderer::FlushFrame()
{
	glDrawPixels(m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_default_buffer->get_attachment(0)->get_bits());
}

void Renderer::FlushFrame(FrameBuffer* fb, int attachment)
{
	auto texture2d = fb->get_attachment(attachment);
	glDrawPixels(fb->get_width(),fb->get_height(), GL_RGBA, GL_UNSIGNED_BYTE, texture2d->get_bits());

}

void Renderer::Fun(int min_y, int max_y)
{
	for (int y = min_y; y < max_y; y++)
	{
		for (int x = lines[y][0]; x <= lines[y][1]; x++)
		{
			auto& tasks = temp_buffer[y * m_width + x];
			int count = tasks.trangle_count;
			if (count == 0) continue;
			tasks.trangle_count = 0;

			auto pre_depth = m_buffer->get_depth_attachment()->get_depth(x, y);
			
			for (int i = 0; i < count; i++)
			{
				auto t_id = tasks.triangle_ids_[i];
				auto& tri_context = m_triangle_context[t_id];

				auto p0 = tri_context.ordered_vs_[0]->coords_;
				auto p1 = tri_context.ordered_vs_[1]->coords_;
				auto p2 = tri_context.ordered_vs_[2]->coords_;

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
				if (depth > pre_depth)continue;
				pre_depth = depth;
				float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

				//透视正确插值
				float c0 = tri_context.ordered_vs_[0]->rhw_ * a * w;
				float c1 = tri_context.ordered_vs_[1]->rhw_ * b * w;
				float c2 = tri_context.ordered_vs_[2]->rhw_ * c * w;
			}
			m_buffer->get_depth_attachment()->set_depth(x,y,pre_depth);
			m_buffer->get_attachment(0)->set_pixel(x, y, Color(0xff,0xff,0xff,0xff));

		}
	}
}
