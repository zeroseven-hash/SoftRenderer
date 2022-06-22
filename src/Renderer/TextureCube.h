#pragma once
#include"Texture.h"


class TextureCube
{
public:
	TextureCube() = default;
	TextureCube(const char* filepath)
	{
		//currently only support dds;
		LoadDDS(filepath);
	}

	~TextureCube()
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_textures[i]) delete m_textures[i];
		}
	}
	static std::shared_ptr<TextureCube> Create(const char* filepath)
	{
		return std::make_shared<TextureCube>(filepath);
	}

	TinyMath::Vec4f SamplerCube(const TinyMath::Vec3f& uvw);
	TinyMath::Vec4f SamplerCubeLod(const TinyMath::Vec3f& uvw, float lod);
private:

	void LoadDDS(const char* filename);
private:

	//0 is X,1 is -X,2 is Y,3 is -Y,4 is Z,5 is -Z 
	Texture2D* m_textures[6]={nullptr};
	uint32_t m_layers;
	TextureFormat m_format;
};