#include"Renderer.h"

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
	m_vertex_shader = nullptr;
	m_fragment_shader = nullptr;

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
	m_color_bg = Color(0x00,0x00,0xff,0xff);
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
