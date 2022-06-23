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
//TinyMath::Vec4f TextureCube::SamplerCube(const TinyMath::Vec3f& uvw)
//{
//	return SamplerCubeLod(uvw, 0.0f);
//}
//TinyMath::Vec4f TextureCube::SamplerCubeLod(const TinyMath::Vec3f& uvw, float lod)
//{
//	auto a=std::max({ 1,2,3 });
//
//}
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
