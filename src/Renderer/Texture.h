#pragma once

#include"Math.h"

#include<stdint.h>
#include<vector>
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


enum class TextureFormat
{
	RGB8 = 0,
	RGBA8,
	RBG32F,
	RGBA32F,
	UNKNOWN
};

class Texture
{
public:
	virtual ~Texture() { if (m_bits) delete[] m_bits; m_bits = nullptr; }
	const uint8_t* get_bits()const { return m_bits; }
	
protected:
	Texture() = default;
	Texture(TextureLayout layout = TextureLayout::LINEAR):m_layout(layout){}
protected:
	uint8_t* m_bits=nullptr;
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
	Texture2D(const char* filepath, TextureLayout layout = TextureLayout::LINEAR):Texture(layout)
	{
		LoadFile(filepath);
	}
	Texture2D(uint32_t width, uint32_t height,TextureFormat format,TextureLayout layout=TextureLayout::LINEAR,TextureFlag_ texture_flag = 0);
	

	Texture2D(const Texture2D& texture) = delete;

	~Texture2D() 
	{
		ClearMipMem();
	};
	void Resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		if (m_layout == TextureLayout::TILED)
		{
			m_width_in_tiles = (m_width + s_tile_w - 1) / s_tile_w;
			int rest = m_width % s_tile_w;
			m_width += (rest == 0) ? 0 : (s_tile_w - rest);

			rest = m_height % s_tile_h;
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
	void Built(const uint8_t* bits, size_t size)
	{
		memcpy(m_bits, bits, size);
	}
	void SaveBMPFile(const char* filename);

	void GenerateMipmap();
	TinyMath::Vec4f Sampler2D(const TinyMath::Vec2f& uv)const;
	TinyMath::Vec4f Texture2D::Sampler2DLod(const TinyMath::Vec2f& uv,float lod)const;


	/*
	* width:width
	* height:height
	* layer:mipmap layer
	* flag:sampler flag
	*/
	static std::shared_ptr<Texture2D> Create(uint32_t width,uint32_t height, TextureFormat format,TextureLayout layout = TextureLayout::LINEAR,  TextureFlag_ texture_flag=0)
	{
		return std::make_shared<Texture2D>(width, height,format, layout, texture_flag);
	}
	static std::shared_ptr<Texture2D> Create(const char* filepath, TextureLayout layout = TextureLayout::LINEAR)
	{
		return std::make_shared<Texture2D>(filepath, layout);
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

	void set_mipmaps(std::vector<Texture2D*>&& mipmaps)
	{
		ClearMipMem();
		m_mipmaps = std::move(mipmaps);
	}

	TinyMath::Vec4f get_pixel(int x, int y)const 
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
		switch (m_channel)
		{
		case 3:
		{
			Color color;
			memcpy(&color, m_bits+index, sizeof(color));
			color.w_ = 255;
			return TinyMath::TransformToVec4(color);
		}
		case 4:
		{
			Color color;
			memcpy(&color, m_bits + index, sizeof(color));
			return TinyMath::TransformToVec4(color);
		}
		case 12:
		{
			TinyMath::Vec4f color;
			memcpy(&color, m_bits + index, sizeof(color));
			color.w_ = 1.0f;
			return color;
		}
		case 16:
		{
			TinyMath::Vec4f color;
			memcpy(&color, m_bits + index, sizeof(color));
			return color;	
		}

		default: {assert(false); break; }
		}
		return TinyMath::Vec4f();
	}

	uint32_t get_width()const { return m_width; }
	uint32_t get_height()const { return m_height; }

private:
		void ClearMipMem()
		{

			//mimaps[0] is self;
			for (uint32_t i = 1; i < m_mipmaps.size(); i++)
			{
				delete m_mipmaps[i];
				m_mipmaps[i] = nullptr;
			}
			m_mipmaps.clear();
		}
		void LoadFile(const char* filename, TextureFlag_ texture_flag = 0);
		void SetFilterAndMode(TextureFlag_ texture_flag)
		{
			if (texture_flag & SAMPLER_NEARST) m_filter = Filter::NEARST;
			else if (texture_flag * SAMPLER_LINEAR) m_filter = Filter::LINEAR;

			if (texture_flag & SAMPLER_REPEAT) m_mode = ClipMode::REPEAT;
			else if (texture_flag & SAMPLER_CLAMP_TO_DEGE) ClipMode::CLAMP_TO_EDGE;
			else if (texture_flag & SAMPLER_CLAMP_TO_BORDER) ClipMode::CLAMP_TO_BORDER;
		}
private:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_channel;
	uint32_t m_pitch;			//pitch=width*channel;
	TextureFormat m_format;

	std::vector<Texture2D*> m_mipmaps=std::vector<Texture2D*>();
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

	void BuiltAll(DepthAttachment& other)
	{
		assert(other.m_width == m_width && other.m_height == m_height);
		memcpy(other.m_depth, m_depth, m_width*m_height*sizeof(float));
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