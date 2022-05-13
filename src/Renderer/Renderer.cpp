#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include"Renderer.h"

#include"Timer.h"
#include"stb_image.h"
#include"stb_image_write.h"


void Canvas::LoadFile(const char* filename)
{
	if (!m_bits)
		delete[] m_bits;
	stbi_set_flip_vertically_on_load(false);
	m_bits = stbi_load(filename, &m_width, &m_height, &m_channel, 0);
}

void Canvas::SaveFileBMP(const char* filename)
{
	stbi_write_bmp(filename, m_width, m_height, m_channel, m_bits);
}

Color Canvas::Sample2D(const TinyMath::Vec2f& tex_coords)
{
	//默认连续平铺纹理
	
	
	TinyMath::Vec2f tex;
	tex.x_ = (tex_coords.x_-(int)tex_coords.x_) * m_width+0.5;
	tex.y_ = (tex_coords.y_-(int)tex_coords.y_) * m_height+0.5;
	//左下，右下，左上，右上
	struct {
		TinyMath::Vec2i coords_;
		TinyMath::Vec2f pos_;
	}vs[4];

	int integer[2] = { (int)tex.x_ ,(int)tex.y_ };
	for (int i = 0;i < 2;i++)
	{
		if (tex.m_[i] - integer[i] > 0.5f)
		{
			vs[0].pos_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i]) + 0.5f;
			vs[0].coords_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i]);
		}
		else
		{
			vs[0].pos_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i] - 1) + 0.5f;
			vs[0].coords_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i] - 1);
		}
	}

	int dir[3][2] = { {1,0},{0,1},{1,1} };
	for (int i = 1;i < 4;i++)
	{
		vs[i].pos_.x_ = vs[0].pos_.x_ + dir[i-1][0];
		vs[i].pos_.y_ = vs[0].pos_.y_ + dir[i-1][1];

		vs[i].coords_.x_ = vs[0].coords_.x_ + dir[i-1][0];
		vs[i].coords_.y_ = vs[0].coords_.y_ + dir[i-1][1];
	}

	Color color[4];
	for (int i = 0;i < 4;i++)
	{
		color[i] = get_pixel(vs[i].coords_.x_, vs[i].coords_.y_);
	}
	float dy1x1x2 = tex.x_ - vs[0].pos_[0];
	float dy2x1x2 = tex.x_ - vs[2].pos_[0];
	Color c1 = LinearInterp(color[0], color[1], dy1x1x2);
	Color c2 = LinearInterp(color[2], color[3],dy2x1x2);

	float dy1dy2 = tex.y_ - vs[0].pos_[1];
	Color res = LinearInterp(c1, c2, dy1dy2);
	return res;
}
void Renderer::Init(int width, int height)
{
	Reset();
	m_canvas = new Canvas(width, height);
	m_width = width;
	m_height = height;
	m_depth_buffer = new float* [height];
	for (int i = 0; i < height; i++) m_depth_buffer[i] = new float[width];
	Clear(DEPTH_BUFFER_BIT | COLOR_BUFFER_BIT);
}

void Renderer::Reset()
{

	if (m_depth_buffer)
	{
		for (int i = 0; i < m_height; i++)
		{
			if (m_depth_buffer[i]) delete[] m_depth_buffer[i];
				m_depth_buffer[i] = nullptr;
		}
		delete[] m_depth_buffer;
		m_depth_buffer = nullptr;
	}
	if (m_canvas) delete m_canvas;
	m_canvas = nullptr;
	m_color_fg = Color(0xff,0xff,0xff,0xff);
	m_color_bg = Color(0x00,0x00,0x00,0xff);
}

void Renderer::Clear(BufferFlag flag)
{
	if (flag & DEPTH_BUFFER_BIT)
	{
		if (m_depth_buffer)
		{
			for (int i= 0; i < m_height; i++) 
			{
				for (int j = 0; j < m_width; j++)
					m_depth_buffer[i][j] = 0.0f;
			}
		}
	}

	if (flag & COLOR_BUFFER_BIT)
	{
		if (m_canvas) m_canvas->Fill(m_color_bg);
	}
}
void Canvas::DrawLine(int x1, int y1, int x2, int y2, const Color& color)
{
	//bresenham 画线算法，由于x，y都在第一象限，所以只有两种情况
	int x, y;
	if (x1 == x2 && y1 == y2) {
		set_pixel(x1, y1, color);
		return;
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) set_pixel(x1, y, color);
		set_pixel(x2, y2, color);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) set_pixel(x, y1, color);
		set_pixel(x2, y2, color);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		int rem = 0;
		if (dx >= dy) {	//slope <1;
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++) {
				set_pixel(x, y, color);
				rem += dy;
				if (rem >= dx) { rem -= dx; y += (y2 >= y1) ? 1 : -1; set_pixel(x, y, color); }
			}
			set_pixel(x2, y2, color);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++) {
				set_pixel(x, y, color);
				rem += dx;
				if (rem >= dy) { rem -= dy; x += (x2 >= x1) ? 1 : -1; set_pixel(x, y, color); }
			}
			set_pixel(x2, y2, color);
		}
	}
}
