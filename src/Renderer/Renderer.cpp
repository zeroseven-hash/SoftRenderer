#include"Renderer.h"

#include<algorithm>

#include"Timer.h"
#include<glad/glad.h>


Vertex Renderer::m_vertex[4][3];

FrameBuffer* Renderer::m_temp_buffer[4];
FrameBuffer* Renderer::m_buffer;
FrameBuffer* Renderer::m_default_buffer;

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
	for (int i = 0; i < 4; i++)
	{
		m_temp_buffer[i] = new FrameBuffer(width, height, spec);
	}
	m_default_buffer = m_temp_buffer[0];
	m_color_bg = Color(0x00, 0x00, 0x00, 0x00);
	m_color_fg = Color(0xff, 0xff, 0xff, 0xff);
	m_render_flag = DRAW_PIXEL;
	m_buffer = m_default_buffer;
	glViewport(0,0,width, height);
}

void Renderer::Clear(BufferFlag flag, const Color& color, float depth)
{
	std::future<void> res[4];
	for (int i = 0; i < 4; i++)
	{
		res[i]=m_thread_pool.PushTask(std::bind(&FrameBuffer::Clear, m_temp_buffer[i], std::placeholders::_1, std::placeholders::_2), color, depth);
	}

	m_buffer->Clear(color,depth);
	for (int i = 0; i < 4; i++) res[i].wait();
}

void Renderer::Bind(FrameBuffer* fb)
{
	m_buffer = fb;
}

void Renderer::UnBind()
{
	m_buffer = m_default_buffer;
}

void Renderer::SetViewPort(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	for (int i = 0; i < 4; i++) m_temp_buffer[i]->Resize(width, height);
	glViewport(0, 0, width, height);
}

void Renderer::FlushFrame()
{
	glDrawPixels(m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_default_buffer->get_attachment(0)->get_bits());
}

void Renderer::FlushFrame(FrameBuffer* fb, int attachment)
{
	m_thread_pool.Join();
	auto texture2d = fb->get_attachment(attachment);
	glDrawPixels(fb->get_width(),fb->get_height(), GL_RGBA, GL_UNSIGNED_BYTE, texture2d->get_bits());

}
