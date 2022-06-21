#include"Renderer.h"

#include<algorithm>

#include"Timer.h"
#include<glad/glad.h>



FrameBuffer* Renderer::m_buffer;


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

