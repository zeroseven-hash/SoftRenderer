#pragma once
#include"Texture.h"

#include<stb_image.h>


template<typename Drive, typename T>
class BaseLayout
{
public:
	using Type = typename T;
	void* AllocateMem(uint32_t width, uint32_t height, uint32_t channel)
	{
		return static_cast<Drive*>(this)->AllocateMem(width, height, channel);
	}


	void set_pixel(int x, int y, Type* color, uint32_t count)
	{
		int index = CalculateIndex(x, y);
		memcpy((Type*)m_bits + index, color, sizeof(Type) * count);
	}
	void get_pixel(int x,int y,Type* color,uint32_t count)const
	{
		int index = CalculateIndex(x, y);
		memcpy(color, (Type*)m_bits + index, sizeof(Type) * count);
	}

	void Fill(Type* color, uint32_t cout)
	{
		int width = m_width;
		int height = m_height;
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				set_pixel(x, y, color, cout);
			}
		}
	}
	int CalculateIndex(int x, int y)const
	{
		return static_cast<const Drive*>(this)->CalculateIndex(x, y);
	}
	void DestoryMem()
	{
		if (m_bits) delete[] m_bits;
	}
	void* CalculateEnd()
	{
		return (void*)((Type*)m_bits + m_height * m_pitch);
	}
protected:
	void* m_bits = nullptr;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_channel;
	uint32_t m_pitch;		//pitch =width*channel
};
template<typename T>
class LinearLayout :public BaseLayout<LinearLayout<T>, T>
{
public:
	void* AllocateMem(uint32_t width, uint32_t height, uint32_t channel)
	{
		m_width = width;
		m_height = height;
		m_channel = channel;
		m_pitch = width * channel;
		m_bits = (void*)new T[m_height * m_pitch];

		return m_bits;
	}

	int CalculateIndex(int x, int y)const
	{
		return y * m_pitch + x * m_channel;
	}
};

template<typename T>
class TileLayout :public BaseLayout<TileLayout<T>, T>
{
public:
	void* AllocateMem(uint32_t width, uint32_t height, uint32_t channel)
	{
		//s_tile_w is 2^n, a%s_tile_w==a&(s_tile_w-1);
		int rest = width & (s_tile_w - 1);
		width += (rest == 0) ? 0 : (s_tile_w - rest);
		rest = height & (s_tile_w - 1);

		height += (rest == 0) ? 0 : (s_tile_h - rest);
		m_height = height;
		m_width = width;
		m_channel = channel;

		m_width_in_tiles = (m_width + s_tile_w - 1) / s_tile_w;
		m_tiles_pitch = s_tile_w * m_channel;

		m_pitch = m_width * m_channel;

		m_bits = (void*) new T[m_height * m_pitch];
		return m_bits;
	}
	int CalculateIndex(int x, int y)const
	{
		uint32_t tilex = x / s_tile_w;
		uint32_t tiley = y / s_tile_h;
		uint32_t in_tilex = x % s_tile_w;
		uint32_t in_tiley = y % s_tile_h;
		int index = (tiley * m_width_in_tiles + tilex) * (m_tiles_pitch * s_tile_h) + in_tiley * m_tiles_pitch + in_tilex * m_channel;
		return index;
	}

private:
	//only for TextureLayout::Tiled
	const int s_tile_w = 4;
	const int s_tile_h = 4;
	int m_tiles_pitch;
	int m_width_in_tiles;
};




template<typename Layout = LinearLayout<uint8_t>>
class TextureDrive :public Texture2D
{
	using Type = typename Layout::Type;
	using Type_Ptr = typename Layout::Type*;
public:
	TextureDrive(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR);
	TextureDrive(const char* filename, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR);
	~TextureDrive() { m_layout_strategy.DestoryMem(); m_bits = nullptr; }

	virtual void Resize(uint32_t width, uint32_t height)override;
	virtual void Fill(const Color& Color)override;
	virtual void Fill(const TinyMath::Vec4f& color)override;
	virtual void Fill(float value)override;
	virtual void set_pixel(int x, int y, const TinyMath::Vec4f& color)override;
	virtual void set_pixel(int x, int y, const Color& color)override;
	virtual TinyMath::Vec4f get_pixel(int x, int y)const override;


	float get_depth(int x, int y)const;
	void set_depth(int x, int y, float depth);
private:
	void LoadFile(const char* filename);
private:
	Layout m_layout_strategy;
};

#include"TextureDrive.inl"
using DepthAttachment = TextureDrive<LinearLayout<float>>;






