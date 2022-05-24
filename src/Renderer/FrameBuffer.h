#pragma once

#include<vector>

#include"Texture.h"


struct TextureSpec
{

	TextureLayout layout_=TextureLayout::LINEAR;	
	int layer_=0;
	TextureFlag_ flag_=0;
};
struct TextureSpecList
{
public:
	TextureSpecList(const std::initializer_list<TextureSpec>& specs)
		:m_specs(specs){}
public:
	std::vector<TextureSpec> m_specs;
};
class FrameBuffer
{
public:
	FrameBuffer() = delete;
	FrameBuffer(uint32_t width, uint32_t height,TextureSpecList specs, bool depth = true)
		:m_width(width),m_height(height),m_depth_flag(depth)
	{
		m_tex_attachments.reserve(specs.m_specs.size());
		for (int i = 0; i < specs.m_specs.size(); i++)
		{
			auto& spec = specs.m_specs[i];
			Texture2D* tex = new Texture2D(width, height, spec.layout_, spec.layer_, spec.flag_);
			m_tex_attachments.emplace_back(tex);
		}

		if (m_depth_flag)
		{
			m_depth_attachment = new DepthAttachment(width, height);
		}
		else m_depth_attachment = nullptr;
	}

	~FrameBuffer()
	{
		ClearMem();
	}
	

	void Resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		for (auto tex : m_tex_attachments) tex->Resize(width, height);
		if (m_depth_flag) m_depth_attachment->Resize(width, height);
		
	}
	void Clear(const Color& color, float depth=1.0f)
	{
		for (int i = 0; i < m_tex_attachments.size(); i++)
		{
			m_tex_attachments[i]->Fill(color);
		}
		if (m_depth_flag) m_depth_attachment->Fill(depth);
	}
	
	void ClearMem()
	{
		for (auto tex : m_tex_attachments)
		{
			if (tex) delete tex;
			tex = nullptr;
		}
		if (m_depth_flag) 
		{
			delete m_depth_attachment; 
			m_depth_attachment = nullptr;
		}
	}
public:
	Texture2D* get_attachment(int index)
	{
		assert(index < m_tex_attachments.size());
		return m_tex_attachments[index];
	}
	DepthAttachment* get_depth_attachment()
	{
		return m_depth_attachment;
	}
	uint32_t get_width()const { return m_width; }
	uint32_t get_height()const { return m_height; }
private:
	uint32_t m_width;
	uint32_t m_height;

	std::vector<Texture2D*> m_tex_attachments;
	bool m_depth_flag;
	DepthAttachment* m_depth_attachment;
};