#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include"Texture.h"

#include<algorithm>

#include"stb_image.h"
#include"stb_image_write.h"

Texture2D::Texture2D(uint32_t width, uint32_t height,TextureLayout layout, int layer, TextureFlag_ texture_flag_)
	:m_width(width),m_height(height),Texture(layout)
{
	
	if (texture_flag_ & SAMPLER_LINEAR) m_filter = Filter::NEARST;
	else if (texture_flag_ * SAMPLER_LINEAR) m_filter = Filter::LINEAR;
	
	if (texture_flag_ & SAMPLER_REPEAT) m_mode = ClipMode::REPEAT;
	else if (texture_flag_ & SAMPLER_CLAMP_TO_DEGE) ClipMode::CLAMP_TO_EDGE;
	else if (texture_flag_ & SAMPLER_CLAMP_TO_BORDER) ClipMode::CLAMP_TO_BORDER;

	m_channel = 4;
	if (m_layout==TextureLayout::TILED)
	{
		int rest = m_width % s_tile_w;
		m_width += (rest == 0) ? 0 : (s_tile_w - rest);

		rest = m_height & s_tile_h;
		m_height += (rest == 0) ? 0 : (s_tile_h - rest);
		m_width_in_tiles = (m_width + s_tile_w - 1) / s_tile_w;
		m_tiles_pitch = s_tile_w * m_channel;
	}
	m_pitch = m_width * m_channel;
	m_bits = new uint8_t[m_pitch * m_height];
	Fill(Color(0xff, 0xff, 0xff, 0xff));
}

void Texture2D::Fill(const Color& color) 
{
	uint8_t* end = m_bits + m_pitch * m_height;
	std::fill((uint32_t*)m_bits, (uint32_t*)end, *(uint32_t*)&color);
}

void Texture2D::LoadFile(const char* filename, TextureFlag_ texture_flag_ )
{
	if (!m_bits)
		delete[] m_bits;
	stbi_set_flip_vertically_on_load(false);
	uint8_t* data = stbi_load(filename,(int*) & m_width,(int*) & m_height,(int*) & m_channel, 0);
	if (m_layout == TextureLayout::TILED)
	{
		int width = m_width;
		int height = m_height;
		int rest = m_width % s_tile_w;
		m_width += (rest == 0) ? 0 : (s_tile_w - rest);

		rest = m_height & s_tile_h;
		m_height += (rest == 0) ? 0 : (s_tile_h - rest);
		m_width_in_tiles = (m_width + s_tile_w - 1) / s_tile_w;
		m_tiles_pitch = s_tile_w * m_channel;

		m_pitch = m_width * m_channel;
		m_bits = new uint8_t[m_pitch * m_height];
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				Color color;
				memcpy(&color, data + y * width * m_channel + x * m_channel, sizeof(color));
				set_pixel_no_cache(x, y, color);
			}
		}

		stbi_image_free(data);
	}
	else
	{
		m_pitch = m_width * m_channel;
		m_bits = data;
	}
	if (texture_flag_ & SAMPLER_NEARST) m_filter = Filter::NEARST;
	else if (texture_flag_ * SAMPLER_LINEAR) m_filter = Filter::LINEAR;

	if (texture_flag_ & SAMPLER_REPEAT) m_mode = ClipMode::REPEAT;
	else if (texture_flag_ & SAMPLER_CLAMP_TO_DEGE) ClipMode::CLAMP_TO_EDGE;
	else if (texture_flag_ & SAMPLER_CLAMP_TO_BORDER) ClipMode::CLAMP_TO_BORDER;
}

void Texture2D::SaveBMPFile(const char* filename)
{
	stbi_write_bmp(filename, m_width, m_height, m_channel, m_bits);
}

Color Texture2D::Sampler2D(const TinyMath::Vec2f& uv)const
{
	//cal uv
	auto norm_uv = uv;
	switch (m_mode)
	{
	case ClipMode::REPEAT:
	{
		norm_uv.u_ -= std::floor(norm_uv.u_);
		norm_uv.v_ -= std::floor(norm_uv.v_);
		break;
	}
	case ClipMode::CLAMP_TO_EDGE:
	{
		for (int i = 0; i < 2; i++)
		{
			if (norm_uv[i] > 1.0f) norm_uv[i] = 1.0f;
			if (norm_uv[i] < 0.0f) norm_uv[i] = 0.0f;
		}
		break;
	}
	case ClipMode::CLAMP_TO_BORDER:
	{
		for (int i = 0; i < 2; i++)
		{
			if (norm_uv[i] > 1.0f || norm_uv[i] < 0.0f) return Color{ 0xff,0xff,0xff,0xff };
		}
		break;
	}
	default: break;
	}

	//filter
	switch (m_filter)
	{
	case Filter::NEARST:
	{
		norm_uv.u_ *= (m_width - 1);
		norm_uv.v_ *= (m_height - 1);
		int x = (int)norm_uv.u_;
		int y = (int)norm_uv.v_;

		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		return get_pixel(x, y);
	}
	case Filter::LINEAR:
	{
		norm_uv.u_ *= (m_width - 1);
		norm_uv.v_ *= (m_height - 1);
		int x = (int)norm_uv.u_;
		int y = (int)norm_uv.v_;


		float dx = norm_uv.u_ - x;
		float dy = norm_uv.v_ - y;

		int left = x;
		int right = TinyMath::Between(0, (int)m_width - 1, x + 1);
		int up = y;
		int down = TinyMath::Between(0, (int)m_height - 1, y + 1);

		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		Color c1 = get_pixel(left, up);
		Color c2 = get_pixel(right, up);
		c1=TinyMath::LinerInterpolation(c1, c2, dx);


		Color c3 = get_pixel(left, down);
		Color c4 = get_pixel(right, down);

		c2 = TinyMath::LinerInterpolation(c3, c4, dx);
		
		
		Color res = TinyMath::LinerInterpolation(c1, c2, dy);
		return res;
	}
	default: break;
	}
	


	
}

void DepthAttachment::Fill(float value)
{
	std::fill(m_depth,m_depth + m_width * m_height, value);
}
