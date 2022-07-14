#pragma once
#define STB_IMAGE_IMPLEMENTATION
//#include"Texture.h"
#include"TextureDrive.h"

uint32_t Texture2D::FormatToChannel(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA32F:
		return 4;
	case TextureFormat::RGB8: 
	case TextureFormat::RGB32F:
		return 3;
	case TextureFormat::DEPTH32F: 
		return 1;
	default: {assert(false); break; }
	}
	return 0;
}

Texture2DHandle Texture2D::CreateRef(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag, TextureLayout layout)
{
	if (layout == TextureLayout::LINEAR)
	{
		switch (format)
		{
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
			return std::make_shared<TextureDrive<LinearLayout<uint8_t>>>(width, height, format, flag, layout);
		case TextureFormat::RGB32F:
		case TextureFormat::RGBA32F:
		case TextureFormat::DEPTH32F:
			return std::make_shared<TextureDrive<LinearLayout<float>>>(width, height, format, flag, layout);
		case TextureFormat::UNKNOWN:
			assert(false);
		}
	}
	else
	{
		switch (format)
		{
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
			return std::make_shared<TextureDrive<TileLayout<uint8_t>>>(width, height, format, flag, layout);
		case TextureFormat::RGB32F:
		case TextureFormat::RGBA32F:
		case TextureFormat::DEPTH32F:
			return std::make_shared<TextureDrive<TileLayout<float>>>(width, height, format, flag, layout);
		case TextureFormat::UNKNOWN:
			assert(false);
		}
	}
	return nullptr;
}

Texture2DHandle Texture2D::CreateRef(const char* filename, TextureFlag_ flag, TextureLayout layout)
{
	const char* sub = strstr((char*)filename, ".hdr");

	if (layout == TextureLayout::LINEAR)
	{

		if (sub) return std::make_shared<TextureDrive<LinearLayout<float>>>(filename, flag, layout);
		else return std::make_shared<TextureDrive<LinearLayout<uint8_t>>>(filename, flag);
	}
	else
	{
		if (sub) return std::make_shared<TextureDrive<TileLayout<float>>>(filename, flag, layout);
		else return std::make_shared<TextureDrive<TileLayout<uint8_t>>>(filename, flag, layout);
	}
	return nullptr;
}

Texture2D* Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format, TextureFlag_ flag, TextureLayout layout)
{
	if (layout == TextureLayout::LINEAR)
	{
		switch (format)
		{
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
			return new TextureDrive<LinearLayout<uint8_t>>(width, height, format, flag, layout);
		case TextureFormat::RGB32F:
		case TextureFormat::RGBA32F:
		case TextureFormat::DEPTH32F:
			return new TextureDrive<LinearLayout<float>>(width, height, format, flag, layout);
		case TextureFormat::UNKNOWN:
			assert(false);
		}
	}
	else
	{
		switch (format)
		{
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
			return new TextureDrive<TileLayout<uint8_t>>(width, height, format, flag, layout);
		case TextureFormat::RGB32F:
		case TextureFormat::RGBA32F:
		case TextureFormat::DEPTH32F:
			return new TextureDrive<TileLayout<float>>(width, height, format, flag, layout);
		case TextureFormat::UNKNOWN:
			assert(false);
		}
	}
	return nullptr;
}

Texture2D* Texture2D::Create(const char* filename, TextureFlag_ flag, TextureLayout layout)
{
	const char* sub = strstr((char*)filename, ".hdr");

	if (layout == TextureLayout::LINEAR)
	{

		if (sub) new TextureDrive<LinearLayout<float>>(filename, flag, layout);
		else new TextureDrive<LinearLayout<uint8_t>>(filename, flag);
	}
	else
	{
		if (sub) new TextureDrive<TileLayout<float>>(filename, flag, layout);
		else new TextureDrive<TileLayout<uint8_t>>(filename, flag, layout);
	}
	return nullptr;
}


TinyMath::Vec4f Texture2D::Sampler2D(const TinyMath::Vec2f& uv) const
{
	//cal uv
	auto norm_uv = uv;
	switch (m_wrapper)
	{
	case Wrapper::REPEAT:
	{
		norm_uv.u_ -= std::floor(norm_uv.u_);
		norm_uv.v_ -= std::floor(norm_uv.v_);
		break;
	}
	case Wrapper::CLAMP_TO_EDGE:
	{
		for (int i = 0; i < 2; i++)
		{
			if (norm_uv[i] > 1.0f) norm_uv[i] = 1.0f;
			if (norm_uv[i] < 0.0f) norm_uv[i] = 0.0f;
		}
		break;
	}
	case Wrapper::CLAMP_TO_BORDER:
	{
		for (int i = 0; i < 2; i++)
		{
			if (norm_uv[i] > 1.0f || norm_uv[i] < 0.0f) return TinyMath::Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	}
	default: assert(false);
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

		assert((uint32_t)x >= 0 && (uint32_t)x < m_width);
		assert((uint32_t)y >= 0 && (uint32_t)y < m_height);

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

		assert((uint32_t)x >= 0 && (uint32_t)x < m_width);
		assert((uint32_t)y >= 0 && (uint32_t)y < m_height);

		auto c1 = get_pixel(left, up);
		auto c2 = get_pixel(right, up);
		c1 = TinyMath::LinerInterpolation(c1, c2, dx);


		auto c3 = get_pixel(left, down);
		auto c4 = get_pixel(right, down);

		c2 = TinyMath::LinerInterpolation(c3, c4, dx);
		return TinyMath::LinerInterpolation(c1, c2, dy);
	}
	default:assert(false);
	}
	return TinyMath::Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
}
TinyMath::Vec4f Texture2D::Sampler2DLod(const TinyMath::Vec2f& uv, float lod) const
{
	int lower_lod = (int)std::floor(lod);
	int up_lod = (int)std::ceil(lod);
	lower_lod = TinyMath::Between(0, (int)m_mipmaps.size() - 1, lower_lod);
	up_lod = TinyMath::Between(0, (int)m_mipmaps.size() - 1, up_lod);
	if (lower_lod == up_lod)
	{
		return m_mipmaps[lower_lod]->Sampler2D(uv);
	}
	else
	{
		float t = lod - lower_lod;
		TinyMath::Vec4f color1 = m_mipmaps[lower_lod]->Sampler2D(uv);
		TinyMath::Vec4f color2 = m_mipmaps[up_lod]->Sampler2D(uv);
		return TinyMath::LinerInterpolation(color1, color2, t);
	}
}
void Texture2D::GenerateMipmap()
{
	ClearMipmem();
	m_mipmaps.emplace_back(this);
	if (m_bits == nullptr) { assert(false); }
	uint32_t mip_width = m_width;
	uint32_t mip_height = m_height;
	int last_level = 0;
	while (mip_width != 1 && mip_height != 1)
	{
		mip_width = mip_width >> 1;
		mip_height = mip_height >> 1;

		Texture2D* tex = Create(mip_width, mip_height, m_format, (int)m_wrapper | (int)m_filter, m_layout);

		//may have bug when tex is not square
		for (uint32_t y = 0; y < mip_height; y++)
		{
			for (uint32_t x = 0; x < mip_width; x++)
			{
				auto& last_level_tex = m_mipmaps[last_level];


				auto color0 = last_level_tex->get_pixel(x * 2 + 0, y * 2 + 0);
				auto color1 = last_level_tex->get_pixel(x * 2 + 1, y * 2 + 0);
				auto color2 = last_level_tex->get_pixel(x * 2 + 0, y * 2 + 1);
				auto color3 = last_level_tex->get_pixel(x * 2 + 1, y * 2 + 1);

				TinyMath::Vec4f blender_color = (color0 + color1 + color2 + color3) / 4.0f;
				tex->set_pixel(x, y, blender_color);
			}
		}

		last_level++;
		m_mipmaps.emplace_back(tex);
	}
}
