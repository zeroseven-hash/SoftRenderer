#pragma once
#include"Math.h"

#include<memory>
#include<vector>


typedef int TextureFlag_;
class Texture2D;
using Texture2DHandle = std::shared_ptr<Texture2D>;
enum TextureFlag
{
	SAMPLER_NEARST = BIT(0),
	SAMPLER_LINEAR = BIT(1),
	SAMPLER_REPEAT = BIT(2),
	SAMPLER_CLAMP_TO_DEGE = BIT(3),
	SAMPLER_CLAMP_TO_BORDER = BIT(4)
};

//determine the memory ordered
enum class TextureLayout
{
	LINEAR,
	TILE
};
enum class Wrapper
{
	REPEAT = 0,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER
};

enum class Filter
{
	LINEAR = 0,
	NEARST
};


enum class TextureFormat
{
	RGB8 = 0,
	RGBA8,
	RGB32F,
	RGBA32F,
	DEPTH32F,
	UNKNOWN
};



class Texture2D
{
public:
	TinyMath::Vec4f Sampler2D(const TinyMath::Vec2f& uv)const;
	TinyMath::Vec4f Sampler2DLod(const TinyMath::Vec2f& uv, float lod)const;
	void GenerateMipmap();

	virtual ~Texture2D() {}
	virtual void Resize(uint32_t width, uint32_t height) = 0;
	virtual void Fill(const Color& color) = 0;
	virtual void Fill(const TinyMath::Vec4f& color) = 0;

	//only for depth attachment.
	virtual void Fill(float value) = 0;


	virtual void set_pixel(int x, int y, const Color& color) = 0;
	virtual void set_pixel(int x, int y, const TinyMath::Vec4f& color) = 0;
	virtual TinyMath::Vec4f get_pixel(int x, int y)const = 0;


	static uint32_t FormatToChannel(TextureFormat format);
	static Texture2DHandle CreateRef(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR);
	static Texture2DHandle CreateRef(const char* filename, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR);
	static Texture2D* Create(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR);
	static Texture2D* Create(const char* filename, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR);

	const void* get_bits()const { return m_bits; }
	TextureFormat get_format()const { return m_format; }
	uint32_t get_width()const { return m_width; }
	uint32_t get_height()const { return m_height; }
	void set_mipmaps(std::vector<Texture2D*>&& mipmaps)
	{
		ClearMipmem();
		m_mipmaps = std::move(mipmaps);
	}
protected:
	Texture2D(TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR)
		:m_layout(layout) {
		SetFilterAndWrapper(flag);
	}
	Texture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag = 0, TextureLayout layout = TextureLayout::LINEAR)
		:m_width(width), m_height(height), m_format(format), m_channel(FormatToChannel(format)), m_layout(layout)
	{
		SetFilterAndWrapper(flag);
	}

	void ClearMipmem()
	{
		//mimaps[0] is self;
		for (uint32_t i = 1; i < m_mipmaps.size(); i++)
		{
			delete m_mipmaps[i];
			m_mipmaps[i] = nullptr;
		}
		m_mipmaps.clear();
	}

private:
	void SetFilterAndWrapper(TextureFlag_ flag)
	{
		if (flag & SAMPLER_NEARST)		m_filter = Filter::NEARST;
		else if (flag * SAMPLER_LINEAR) m_filter = Filter::LINEAR;

		if (flag & SAMPLER_REPEAT)				 m_wrapper = Wrapper::REPEAT;
		else if (flag & SAMPLER_CLAMP_TO_DEGE)	 m_wrapper = Wrapper::CLAMP_TO_EDGE;
		else if (flag & SAMPLER_CLAMP_TO_BORDER) m_wrapper = Wrapper::CLAMP_TO_BORDER;
	}

protected:
	void* m_bits = nullptr;
	Filter m_filter = Filter::NEARST;
	Wrapper m_wrapper = Wrapper::REPEAT;

	TextureFormat m_format;
	TextureLayout m_layout;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_channel;


	std::vector<Texture2D*> m_mipmaps;
};

#include"TextureDrive.h"