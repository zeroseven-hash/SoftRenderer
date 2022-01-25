#include"Renderer.h"

void Renderer::Init(int width, int height)
{
	Reset();
	m_canvas = new Canvas(width, height);
	m_width = width;
	m_height = height;
	m_depth_buffer = new float* [height];
	for (int i = 0; i < height; i++) m_depth_buffer[i] = new float[width];
	Clear(DEPTH_BUFFER_BIT | COLOR_BUFFER_BIT);
}

void Renderer::Reset()
{

	if (m_depth_buffer)
	{
		for (int i = 0; i < m_height; i++)
		{
			if (m_depth_buffer[i]) delete[] m_depth_buffer[i];
				m_depth_buffer[i] = nullptr;
		}
		delete[] m_depth_buffer;
		m_depth_buffer = nullptr;
	}
	if (m_canvas) delete m_canvas;
	m_canvas = nullptr;
	m_color_fg = Color(0xff,0xff,0xff,0xff);
	m_color_bg = Color(0x00,0x00,0x00,0xff);
}

void Renderer::Clear(BufferFlag flag)
{
	if (flag & DEPTH_BUFFER_BIT)
	{
		if (m_depth_buffer)
		{
			for (int i= 0; i < m_height; i++) 
			{
				for (int j = 0; j < m_width; j++)
					m_depth_buffer[i][j] = 0.0f;
			}
		}
	}

	if (flag & COLOR_BUFFER_BIT)
	{
		if (m_canvas) m_canvas->Fill(m_color_bg);
	}
}

void Renderer::DrawTriangle(const Shader& shader)
{
	if (m_canvas == nullptr) return;
	for (int k = 0;k < 3;k++)
	{
		Vertex& v = m_vertex[k];
		v.context_.varying_float_.clear();
		v.context_.varying_vec2f_.clear();
		v.context_.varying_vec3f_.clear();
		v.context_.varying_vec4f_.clear();

		v.pos_ = shader.vertex_shader_(k, v.context_);

		//�ü� pos(nx,ny,n^2,n);x y should between [-1,1],n could be near or far
		float w = v.pos_.w_;
		if (w == 0.0f)return;
		if (v.pos_.z_<0.0f || v.pos_.z_>w)return;
		if (v.pos_.x_<-w || v.pos_.x_>w)return;
		if (v.pos_.y_<-w || v.pos_.y_>w)return;


		v.rhw_ = 1.0f / w;
		v.pos_ *= v.rhw_;

		//x:[-1,1] to [0,width-1],y:[-1,1] to [0,height-1];
		//	0.5*width       0	      width*0.5		    x     (x+1.0f)*0.5*width
		//	  0	       height*0.5*   height*0.5    *    y  = (y+1.0f)*0.5*height
		//	  0             0            0	        	1 				1
		v.center_.x_ = (v.pos_.x_ + 1.0f) * m_width * 0.5f;
		//v.center_.y_ = (v.pos_.y_ + 1.0f) * m_height * 0.5f;
		v.center_.y_ = (1.0f-v.pos_.y_) * m_height * 0.5f;

		v.coords_.x_ = (int)(v.center_.x_ + 0.5f);
		v.coords_.y_ = (int)(v.center_.y_ + 0.5f);

		if (k == 0)
		{
			m_min_x = m_max_x = TinyMath::Between(0, m_width - 1, v.coords_.x_);
			m_min_y = m_max_y = TinyMath::Between(0, m_height - 1, v.coords_.y_);
		}
		else
		{
			m_min_x = TinyMath::Between(0, m_width - 1, std::min(m_min_x, v.coords_.x_));
			m_max_x = TinyMath::Between(0, m_width - 1, std::max(m_max_x, v.coords_.x_));
			m_min_y = TinyMath::Between(0, m_height - 1, std::min(m_min_y, v.coords_.y_));
			m_max_y = TinyMath::Between(0, m_height - 1, std::max(m_max_y, v.coords_.y_));
		}
	}
	//TODO:�����߿�
	if (!m_render_pixel) return;

	TinyMath::Vec4f v01 = m_vertex[1].pos_ - m_vertex[0].pos_;
	TinyMath::Vec4f v02 = m_vertex[2].pos_ - m_vertex[0].pos_;
	TinyMath::Vec4f normal = v01.Cross(v02);

	Vertex* vs[3] = { &m_vertex[0],&m_vertex[1] ,&m_vertex[2] };
	if (normal.z_ > 0.0f)
		std::swap(vs[1], vs[2]);
	else if (normal.z_ == 0.0f)
		return;

	TinyMath::Vec2i p[3] = { vs[0]->coords_,vs[1]->coords_ ,vs[2]->coords_ };
	
	// ���������Ϊ����˳�
	float s = std::abs((p[1]-p[0]).Cross(p[2]-p[0]));
	if (s <= 0) return;

	bool TopLeft01 = IsTopLeft(p[0], p[1]);
	bool TopLeft12 = IsTopLeft(p[1], p[2]);
	bool TopLeft20 = IsTopLeft(p[2], p[0]);


	for (int x = m_min_x;x <= m_max_x;x++) for (int y = m_min_y;y <= m_max_y;y++)
	{
		TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };

		// Edge Equation
				// ʹ���������⸡����ͬʱ��Ϊ������ϵ�����Է���ȡ��
		int E01 = -(x - p[0].x_) * (p[1].y_ - p[0].y_) + (y - p[0].y_) * (p[1].x_ - p[0].x_);
		int E12 = -(x - p[1].x_) * (p[2].y_ - p[1].y_) + (y - p[1].y_) * (p[2].x_ - p[1].x_);
		int E20 = -(x - p[2].x_) * (p[0].y_ - p[2].y_) + (y - p[2].y_) * (p[0].x_ - p[2].x_);


		// ��������ϱߣ��� E >= 0 �жϺϷ���������±߾��� E > 0 �жϺϷ�
		// ����ͨ������һ����� 1 ������ < 0 �� <= 0 ��һ��ʽ�ӱ��
		if (E01 < (TopLeft01 ? 0 : 1)) continue;   // �ڵ�һ���ߺ���
		if (E12 < (TopLeft12 ? 0 : 1)) continue;   // �ڵڶ����ߺ���
		if (E20 < (TopLeft20 ? 0 : 1)) continue;   // �ڵ������ߺ���


		// �����˵㵽��ǰ���ʸ��
		TinyMath::Vec2f s0 = vs[0]->center_ - pixel;
		TinyMath::Vec2f s1 = vs[1]->center_ - pixel;
		TinyMath::Vec2f s2 = vs[2]->center_ - pixel;

		// ��������ϵ�������ڲ������������ a / b / c
		float a = std::abs(s1.Cross(s2));    // �������� Px-P1-P2 ���
		float b = std::abs(s2.Cross(s0));    // �������� Px-P2-P0 ���
		float c = std::abs(s0.Cross(s1));    // �������� Px-P0-P1 ���
		float s = a + b + c;                    // �������� P0-P1-P2 ���

		if (s == 0.0f) continue;

		// Barycentric coordinates interpolation
		a = a * (1.0f / s);
		b = b * (1.0f / s);
		c = c * (1.0f / s);

		//����Լ��䵹���ڿ���ֱ��ʹ�����Ĳ�ֵ
		float rhw = vs[0]->rhw_ * a + vs[1]->rhw_ * b + vs[2]->rhw_ * c;
		float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

		//͸����ȷ��ֵ
		float c0 = vs[0]->rhw_ * a * w;
		float c1 = vs[1]->rhw_ * b * w;
		float c2 = vs[2]->rhw_ * c * w;

		ShaderContext input;
		ShaderContext& i0 = vs[0]->context_;
		ShaderContext& i1 = vs[1]->context_;
		ShaderContext& i2 = vs[2]->context_;


		// ��ֵ���� varying
		for (auto const& it : i0.varying_float_) {
			int key = it.first;
			float f0 = i0.varying_float_[key];
			float f1 = i1.varying_float_[key];
			float f2 = i2.varying_float_[key];
			input.varying_float_[key] = c0 * f0 + c1 * f1 + c2 * f2;
		}

		for (auto const& it : i0.varying_vec2f_) {
			int key = it.first;
			const TinyMath::Vec2f& f0 = i0.varying_vec2f_[key];
			const TinyMath::Vec2f& f1 = i1.varying_vec2f_[key];
			const TinyMath::Vec2f& f2 = i2.varying_vec2f_[key];
			input.varying_vec2f_[key] = c0 * f0 + c1 * f1 + c2 * f2;
		}

		for (auto const& it : i0.varying_vec3f_) {
			int key = it.first;
			const TinyMath::Vec3f& f0 = i0.varying_vec3f_[key];
			const TinyMath::Vec3f& f1 = i1.varying_vec3f_[key];
			const TinyMath::Vec3f& f2 = i2.varying_vec3f_[key];
			input.varying_vec3f_[key] = c0 * f0 + c1 * f1 + c2 * f2;
		}

		for (auto const& it : i0.varying_vec4f_) {
			int key = it.first;
			const TinyMath::Vec4f& f0 = i0.varying_vec4f_[key];
			const TinyMath::Vec4f& f1 = i1.varying_vec4f_[key];
			const TinyMath::Vec4f& f2 = i2.varying_vec4f_[key];
			input.varying_vec4f_[key] = c0 * f0 + c1 * f1 + c2 * f2;
		}

		TinyMath::Vec4f color = shader.fragment_shader_(input);
		m_canvas->set_pixel(x, y, Color(color));
	}

}
