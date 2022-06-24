#include "TextureCube.h"
#define TINYDDSLOADER_IMPLEMENTATION

#include"tinyddsloader.h"


static TextureFormat DDSFormatToTextureFormat(tinyddsloader::DDSFile::DXGIFormat format)
{
	switch (format)
	{
	case tinyddsloader::DDSFile::DXGIFormat::R32G32B32A32_Float:return TextureFormat::RGBA32F;
		break;
	case tinyddsloader::DDSFile::DXGIFormat::R32G32B32_Float:return TextureFormat::RBG32F;
		break;
	case tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UInt:return TextureFormat::RGBA8;
	default:
		break;
	}
}
TinyMath::Vec4f TextureCube::SamplerCube(const TinyMath::Vec3f& uvw)
{
	return SamplerCubeLod(uvw, 0.0f);
}
TinyMath::Vec4f TextureCube::SamplerCubeLod(const TinyMath::Vec3f& uvw, float lod)
{
	float abs_x = std::abs(uvw.x_);
	float abs_y = std::abs(uvw.y_);
	float abs_z = std::abs(uvw.z_);
	float mag = std::max({abs_x,abs_y,abs_z});
	if (mag == abs_x)
	{
		TinyMath::Vec2f coords = { uvw.z_ / mag+1.0f,uvw.y_ / mag+1.0f };
		coords /= 2.0f;
		coords.y_ = 1.0f - coords.y_;
		if (uvw.x_ > 0)
		{
			coords.x_ = 1.0f - coords.x_;
			return m_textures[0]->Sampler2DLod(coords, lod);
		}
		else if (uvw.x_ < 0)
		{

			return m_textures[1]->Sampler2DLod(coords, lod);
		}


	}
	else if (mag == abs_y)
	{
		TinyMath::Vec2f coords = { uvw.x_ / mag + 1.0f,uvw.z_ / mag + 1.0f };
		coords /= 2.0f;
		coords.y_ = 1.0f - coords.y_;
		if (uvw.y_ > 0)
		{
			coords.y_ = 1.0f - coords.y_;
			return m_textures[2]->Sampler2DLod(coords, lod);
		}
		else if (uvw.y_ < 0)
		{
			return m_textures[3]->Sampler2DLod(coords, lod);
		}
	}
	else if (mag == abs_z)
	{
		TinyMath::Vec2f coords = { uvw.x_ / mag + 1.0f,uvw.y_ / mag + 1.0f };
		coords /= 2.0f;
		coords.y_ = 1.0f - coords.y_;
		if (uvw.z_ > 0)
		{
			return m_textures[4]->Sampler2DLod(coords, lod);

		}
		else if (uvw.z_ < 0)
		{
			coords.x_ = 1.0f - coords.x_;
			return m_textures[5]->Sampler2DLod(coords, lod);
		}
	}
}
void TextureCube::LoadDDS(const char* filename)
{
	using namespace tinyddsloader;
	DDSFile dds;
	auto ret = dds.Load(filename);
	if (ret!= tinyddsloader::Result::Success)
	{
		std::cout << "Fail to load TextureCube!\n";
		return;
	}

	if (!dds.IsCubemap())
	{
		std::cout << "File is not TextureCube" << "\n";
	}

	m_layers = dds.GetMipCount();
	m_format = DDSFormatToTextureFormat(dds.GetFormat());
	auto array_size = dds.GetArraySize();
	for (auto t_ptr : m_textures)
	{
		if (t_ptr) delete t_ptr;
		t_ptr = nullptr;
	}
	
	for (uint32_t i = 0; i < 6; i++)
	{
		std::vector<Texture2D*> mipmaps(m_layers);
		for (uint32_t mip_level = 0; mip_level < m_layers; mip_level++)
		{
			const auto* imageData = dds.GetImageData(mip_level,i);
			uint32_t width = imageData->m_width;
			uint32_t height = imageData->m_height;
			mipmaps[mip_level] = new Texture2D(width, height, m_format);
			mipmaps[mip_level]->Built((uint8_t*)imageData->m_mem, imageData->m_memSlicePitch);

		}
		auto tex = mipmaps[0];
		if (m_layers != 1)
		{
			tex->set_mipmaps(std::move(mipmaps));
		}
		m_textures[i] = tex;
	}

}
