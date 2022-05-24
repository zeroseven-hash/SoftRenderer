#pragma once

#include"Math.h"

#include<stdint.h>
#include<memory>
#include <emmintrin.h>


typedef int TextureFlag_;
enum TextureFlag
{
	SAMPLER_NEARST=BIT(0),
	SAMPLER_LINEAR=BIT(1),
	SAMPLER_REPEAT=BIT(2),
	SAMPLER_CLAMP_TO_DEGE=BIT(3),
	SAMPLER_CLAMP_TO_BORDER=BIT(4)
};

//determine the memory ordered
enum class TextureLayout
{
	LINEAR,
	TILED
};
enum class ClipMode
{
	REPEAT=0,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER
};

enum class Filter
{
	LINEAR=0,
	NEARST
};

//Format: RGBA8
class Texture
{
public:
	Texture(TextureLayout layout = TextureLayout::LINEAR):m_layout(layout){}
	virtual ~Texture() { if (m_bits) delete[] m_bits; m_bits = nullptr; }
	const uint8_t* get_bits()const { return m_bits; }
protected:
	uint8_t* m_bits;
	Filter m_filter = Filter::NEARST;
	ClipMode m_mode = ClipMode::REPEAT;
	TextureLayout m_layout;

	//onlyt for TextureLayout::Tiled
	const int s_tile_w = 4;
	const int s_tile_h = 4;
	int m_tiles_pitch;
	int m_width_in_tiles;
};
using TextureHandle = std::shared_ptr<Texture>;

class Texture2D:public Texture
{
public:

	Texture2D(TextureLayout layout=TextureLayout::LINEAR):Texture(layout){}
	Texture2D(uint32_t width, uint32_t height,TextureLayout layout=TextureLayout::LINEAR, int layer = 0, TextureFlag_ texture_flag_ = 0);
	Texture2D(const Texture2D& texture)
		:m_width(texture.m_width), m_height(texture.m_height), m_channel(texture.m_channel), m_pitch(texture.m_pitch)
	{
		m_filter = texture.m_filter;
		m_mode = texture.m_mode;
		m_layout = texture.m_layout;
		m_width_in_tiles = texture.m_width_in_tiles;
		m_tiles_pitch = texture.m_tiles_pitch;
		m_bits = new uint8_t[m_pitch * m_height];
		memcpy(m_bits, texture.m_bits, m_pitch * m_height);
	}

	~Texture2D() = default;
	void Resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		if (m_layout == TextureLayout::TILED)
		{
			m_width_in_tiles = (m_width + s_tile_w - 1) / s_tile_w;
			int rest = m_width % s_tile_w;
			m_width += (rest == 0) ? 0 : (s_tile_w - rest);

			rest = m_height & s_tile_h;
			m_height += (rest == 0) ? 0 : (s_tile_h - rest);

			m_width_in_tiles = (m_width + s_tile_w - 1) / s_tile_w;
			m_tiles_pitch = s_tile_w * m_channel;

		}
		m_pitch = m_width * m_channel;
		if (m_bits) delete m_bits;
		m_bits = new uint8_t[m_pitch * m_height];
		Fill(Color(0xff, 0xff, 0xff, 0xff));
	}
	void Fill(const Color& color);
	void LoadFile(const char* filename,TextureFlag_ texture_flag_=0);
	void SaveBMPFile(const char* filename);

	Color Sampler2D(const TinyMath::Vec2f& uv)const;

	/*
	* width:width
	* height:height
	* layer:mipmap layer
	* flag:sampler flag
	*/
	static std::shared_ptr<Texture2D> Create(uint32_t width,uint32_t height, TextureLayout layout = TextureLayout::LINEAR, int layer=0, TextureFlag_ texture_flag=0)
	{
		return std::make_shared<Texture2D>(width, height, layout,layer, texture_flag);
	}

public:
	void set_pixel(int x, int y, const Color& color)
	{
		size_t index;
		if (m_layout == TextureLayout::TILED)
		{
			uint32_t tilex = x / s_tile_w;
			uint32_t tiley = y / s_tile_h;
			uint32_t in_tilex = x % s_tile_w;
			uint32_t in_tiley = y % s_tile_h;
			index = (tiley * m_width_in_tiles + tilex) * (m_tiles_pitch * s_tile_h) + in_tiley * m_tiles_pitch + in_tilex*m_channel;
		}
		else
			index = y * m_pitch + x * m_channel;
		memcpy(m_bits + index, &color, sizeof(Color));
	}

	 void set_pixel_no_cache(int x, int y, const Color& color)
	{
		 size_t index;
		 if (m_layout == TextureLayout::TILED)
		 {
			 uint32_t tilex = x / s_tile_w;
			 uint32_t tiley = y / s_tile_h;
			 uint32_t in_tilex = x % s_tile_w;
			 uint32_t in_tiley = y % s_tile_h;
			 index = (tiley * m_width_in_tiles + tilex) * (m_tiles_pitch * s_tile_h) + in_tiley * m_tiles_pitch + in_tilex * m_channel;
		 }
		 else
			 index = y * m_pitch + x * m_channel;
		_mm_stream_si32((int*)(m_bits + index), *(int*)&color);
	}

	Color get_pixel(int x, int y)const 
	{
		size_t index;
		if (m_layout == TextureLayout::TILED)
		{
			uint32_t tilex = x / s_tile_w;
			uint32_t tiley = y / s_tile_h;
			uint32_t in_tilex = x % s_tile_w;
			uint32_t in_tiley = y % s_tile_h;
			index = (tiley * m_width_in_tiles + tilex) * (m_tiles_pitch * s_tile_h) + in_tiley * m_tiles_pitch + in_tilex * m_channel;
		}
		else
			index = y * m_pitch + x * m_channel;
		Color color;
		memcpy(&color, m_bits+index, sizeof(color));
		return color;
	}
private:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_channel;

	uint32_t m_pitch;			//row=width*channel;
};


class DepthAttachment
{
public:
	DepthAttachment() = default;
	DepthAttachment(uint32_t width, uint32_t height)
		:m_width(width), m_height(height)
	{
		m_depth = new float[width * height];
	}
	~DepthAttachment() { if (m_depth) delete[] m_depth; }
	void Resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		if (m_depth) delete[] m_depth;
		m_depth = new float[width * height];
		Fill(1.0f);
	}
	float get_depth(int x, int y)
	{
		return m_depth[y * m_width + x];
	}
	void set_depth(int x, int y,float val)
	{
		m_depth[y * m_width + x]=val;
	}
	void Fill(float value);
private:
	uint32_t m_width;
	uint32_t m_height;
	float* m_depth;
};