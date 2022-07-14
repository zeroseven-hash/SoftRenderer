#pragma once
#include"TextureDrive.h"
//implementation
template<typename Layout>
inline TextureDrive<Layout>::TextureDrive(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag, TextureLayout layout)
	:Texture2D(width, height, format, flag, layout)
{
	assert(m_channel != 0);
	m_bits=m_layout_strategy.AllocateMem(m_width, m_height, m_channel);
}

template<typename Layout>
inline TextureDrive<Layout>::TextureDrive(const char* filename, TextureFlag_ flag, TextureLayout layout)
	:Texture2D(flag, layout)
{
	LoadFile(filename);
}



template<typename Layout>
inline void TextureDrive<Layout>::Resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	m_layout_strategy.DestoryMem();
	m_bits = m_layout_strategy.AllocateMem(width, height, m_channel);
}

template<typename Layout>
inline void TextureDrive<Layout>::Fill(const Color& Color)
{
	switch (m_format)
	{
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
		m_layout_strategy.Fill((Type_Ptr)&Color, m_channel); break;
	case TextureFormat::RGB32F:
	case TextureFormat::RGBA32F:
	{
		TinyMath::Vec4f col = TinyMath::TransformToVec4(Color);
		m_layout_strategy.Fill((Type_Ptr)&col, m_channel);
		break;
	}
	default:assert(false);
	}
}

template<typename Layout>
inline void TextureDrive<Layout>::Fill(const TinyMath::Vec4f& color)
{
	switch (m_format)
	{
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	{
		Color col = TinyMath::TransformToColor(color);
		m_layout_strategy.Fill((Type_Ptr)&col, m_channel);
		break;
	}
	case TextureFormat::RGB32F:
	case TextureFormat::RGBA32F:
		m_layout_strategy.Fill((Type_Ptr)&color, m_channel); break;
	default:assert(false);
	}
}

template<typename Layout>
inline void TextureDrive<Layout>::Fill(float value)
{
	assert(false);
}

template<>
inline void TextureDrive<LinearLayout<float>>::Fill(float value)
{
	float* end = (float*)m_layout_strategy.CalculateEnd();
	std::fill((float*)m_bits, end, value);
}
template<>
inline void TextureDrive<TileLayout<float>>::Fill(float value)
{
	float* end = (float*)m_layout_strategy.CalculateEnd();
	std::fill((float*)m_bits, end, value);
}

template<typename Layout>
inline void TextureDrive<Layout>::set_pixel(int x, int y, const TinyMath::Vec4f& color)
{
	switch (m_format)
	{
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	{
		Color col = TinyMath::TransformToColor(color);
		m_layout_strategy.set_pixel(x, y, (Type_Ptr)&col, m_channel);
		break;
	}
	case TextureFormat::RGB32F:
	case TextureFormat::RGBA32F:
	{
		m_layout_strategy.set_pixel(x, y, (Type_Ptr)&color, m_channel); break;
	}
	default:assert(false);
	}
}

template<typename Layout>
inline void TextureDrive<Layout>::set_pixel(int x, int y, const Color& color)
{
	switch (m_format)
	{
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	{
		m_layout_strategy.set_pixel(x, y, (Type_Ptr)&color, m_channel);
		break;
	}
	case TextureFormat::RGB32F:
	case TextureFormat::RGBA32F:
	{
		TinyMath::Vec4f col=TinyMath::TransformToVec4(color);
		m_layout_strategy.set_pixel(x, y, (Type_Ptr)&col, m_channel); break;
	}
	default:assert(false);
	}
}

template<typename Layout>
inline TinyMath::Vec4f TextureDrive<Layout>::get_pixel(int x, int y)const
{
	switch (m_format)
	{
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	{
		Color col;
		m_layout_strategy.get_pixel(x, y, (Type_Ptr)&col, m_channel);
		return TinyMath::TransformToVec4(col);
	}
	case TextureFormat::RGB32F:
	case TextureFormat::RGBA32F:
	{
		TinyMath::Vec4f col;
		m_layout_strategy.get_pixel(x, y, (Type_Ptr)&col, m_channel);
		return col;
	}
	default:assert(false);
	}
}

template<typename Layout>
inline float TextureDrive<Layout>::get_depth(int x, int y) const
{
	assert(false);
}
template<>
inline float TextureDrive<LinearLayout<float>>::get_depth(int x, int y)const
{
	assert(m_channel == 1);
	float d = *((float*)m_bits + y * m_width + x);
	return d;
}


template<typename Layout>
inline void TextureDrive<Layout>::set_depth(int x, int y, float depth)
{
	assert(false);
}

template<>
inline void TextureDrive<LinearLayout<float>>::set_depth(int x, int y, float depth)
{
	*((float*)m_bits + y * m_width + x) = depth;
}

template<typename Layout>
inline void TextureDrive<Layout>::LoadFile(const char* filename)
{
	stbi_set_flip_vertically_on_load(false);
	const char* sub = strstr((char*)filename, ".hdr");
	Type_Ptr data;
	int width;
	int height;
	int channel;
	if (sub)
	{
		data = (Type_Ptr)stbi_loadf(filename, &width, &height, &channel, 0);
		switch (channel)
		{
		case 3: m_format = TextureFormat::RGB32F; break;
		case 4: m_format = TextureFormat::RGBA32F; break;
		default: assert(false);
		}
	}
	else
	{
		auto t = stbi_load(filename, &width, &height, &channel, 0);
		data = (Type_Ptr)stbi_load(filename, &width, &height, &channel, 0);
		switch (channel)
		{
		case 3: m_format = TextureFormat::RGB8; break;
		case 4: m_format = TextureFormat::RGBA8; break;
		default: assert(false);
		}
	}
	m_width = width;
	m_height = height;
	m_channel = channel;

	m_bits = m_layout_strategy.AllocateMem(width, height, channel);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Type_Ptr color = (Type_Ptr)data + y * m_channel * m_width + x * m_channel;
			m_layout_strategy.set_pixel(x, y, color, m_channel);
		}
	}
	stbi_image_free(data);
}