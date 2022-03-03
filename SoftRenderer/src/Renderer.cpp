#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include"Renderer.h"

#include"Timer.h"
#include"stb_image.h"
#include"stb_image_write.h"


void Canvas::LoadFile(const char* filename)
{
	if (!m_bits)
		delete[] m_bits;
	stbi_set_flip_vertically_on_load(true);
	m_bits = stbi_load(filename, &m_width, &m_height, &m_channel, 0);
}

void Canvas::SaveFileBMP(const char* filename)
{
	stbi_write_bmp(filename, m_width, m_height, m_channel, m_bits);
}

Color Canvas::Sample2D(const TinyMath::Vec2f& tex_coords)
{
	
	TinyMath::Vec2f tex;
	tex.x_ = tex_coords.x_ * m_width+0.5;
	tex.y_ = tex_coords.y_ * m_height+0.5;
	//左下，右下，左上，右上
	struct {
		TinyMath::Vec2i coords_;
		TinyMath::Vec2f pos_;
	}vs[4];

	int integer[2] = { (int)tex.x_ ,(int)tex.y_ };
	for (int i = 0;i < 2;i++)
	{
		if (tex.m_[i] - integer[i] > 0.5f)
		{
			vs[0].pos_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i]) + 0.5f;
			vs[0].coords_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i]);
		}
		else
		{
			vs[0].pos_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i] - 1) + 0.5f;
			vs[0].coords_.m_[i] = TinyMath::Between(0, i == 0 ? m_width-1 : m_height-1, integer[i] - 1);
		}
	}

	int dir[3][2] = { {1,0},{0,1},{1,1} };
	for (int i = 1;i < 4;i++)
	{
		vs[i].pos_.x_ = vs[0].pos_.x_ + dir[i-1][0];
		vs[i].pos_.y_ = vs[0].pos_.y_ + dir[i-1][1];

		vs[i].coords_.x_ = vs[0].coords_.x_ + dir[i-1][0];
		vs[i].coords_.y_ = vs[0].coords_.y_ + dir[i-1][1];
	}

	Color color[4];
	for (int i = 0;i < 4;i++)
	{
		color[i] = get_pixel(vs[i].coords_.x_, vs[i].coords_.y_);
	}
	float dy1x1x2 = tex.x_ - vs[0].pos_[0];
	float dy2x1x2 = tex.x_ - vs[2].pos_[0];
	Color c1 = LinearInterp(color[0], color[1], dy1x1x2);
	Color c2 = LinearInterp(color[2], color[3],dy2x1x2);

	float dy1dy2 = tex.y_ - vs[0].pos_[1];
	Color res = LinearInterp(c1, c2, dy1dy2);
	return res;
}
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
void Canvas::DrawLine(int x1, int y1, int x2, int y2, const Color& color)
{
	//bresenham 画线算法，由于x，y都在第一象限，所以只有两种情况
	int x, y;
	if (x1 == x2 && y1 == y2) {
		set_pixel(x1, y1, color);
		return;
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) set_pixel(x1, y, color);
		set_pixel(x2, y2, color);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) set_pixel(x, y1, color);
		set_pixel(x2, y2, color);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		int rem = 0;
		if (dx >= dy) {	//slope <1;
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++) {
				set_pixel(x, y, color);
				rem += dy;
				if (rem >= dx) { rem -= dx; y += (y2 >= y1) ? 1 : -1; set_pixel(x, y, color); }
			}
			set_pixel(x2, y2, color);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++) {
				set_pixel(x, y, color);
				rem += dx;
				if (rem >= dy) { rem -= dy; x += (x2 >= x1) ? 1 : -1; set_pixel(x, y, color); }
			}
			set_pixel(x2, y2, color);
		}
	}
}
void Renderer::DrawTriangle(const Shader& shader, uint32_t* indices)
{
	if (m_canvas == nullptr) return;
	for (int k = 0;k < 3;k++)
	{
		Vertex& v = m_vertex[k];
		v.context_.varying_float_.clear();
		v.context_.varying_vec2f_.clear();
		v.context_.varying_vec3f_.clear();
		v.context_.varying_vec4f_.clear();

		v.pos_ = shader.vertex_shader_(indices[k], v.context_);

		//裁剪 pos(nx,ny,n^2,z);x y should between [-1,1],n could be near or far

		//w_>0;
		float w = v.pos_.w_;
		if (w == 0.0f)return;

		if (v.pos_.z_<-w || v.pos_.z_>w)return;
		if (v.pos_.x_<-w || v.pos_.x_>w)return;
		if (v.pos_.y_<-w || v.pos_.y_>w)return;


		v.rhw_ = 1.0f / w;
		v.pos_ *= v.rhw_;

		//x:[-1,1] to [0,width-1],y:[-1,1] to [0,height-1];
		//	0.5*width       0	      width*0.5		    x     (x+1.0f)*0.5*width
		//	  0	       height*0.5    height*0.5    *    y  = (y+1.0f)*0.5*height
		//	  0             0            0	        	1 				1
		v.center_.x_ = (v.pos_.x_ + 1.0f) * m_width * 0.5f;
		v.center_.y_ = (v.pos_.y_ + 1.0f) * m_height * 0.5f;
		

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
	//TODO:绘制线框
	if (m_render_line)
	{
		m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_,m_color_fg);
		m_canvas->DrawLine(m_vertex[0].coords_.x_, m_vertex[0].coords_.y_, m_vertex[2].coords_.x_, m_vertex[2].coords_.y_,m_color_fg);
		m_canvas->DrawLine(m_vertex[2].coords_.x_, m_vertex[2].coords_.y_, m_vertex[1].coords_.x_, m_vertex[1].coords_.y_,m_color_fg);
	}
	if (!m_render_pixel) return;

	TinyMath::Vec4f v01 = m_vertex[1].pos_ - m_vertex[0].pos_;
	TinyMath::Vec4f v02 = m_vertex[2].pos_ - m_vertex[0].pos_;
	TinyMath::Vec4f normal = v01.Cross(v02);

	Vertex* vs[3] = { &m_vertex[0],&m_vertex[1] ,&m_vertex[2] };
	if (normal.z_ < 0.0f)
		std::swap(vs[1], vs[2]);
	else if (normal.z_ == 0.0f)
		return;

	TinyMath::Vec2i p[3] = { vs[0]->coords_,vs[1]->coords_ ,vs[2]->coords_ };
	
	// 计算面积，为零就退出
	float s = std::abs((p[1]-p[0]).Cross(p[2]-p[0]));
	if (s <= 0) return;

	bool TopLeft01 = IsTopLeft(p[0], p[1]);
	bool TopLeft12 = IsTopLeft(p[1], p[2]);
	bool TopLeft20 = IsTopLeft(p[2], p[0]);


	for (int x = m_min_x;x <= m_max_x;x++) for (int y = m_min_y;y <= m_max_y;y++)
	{
		TinyMath::Vec2f pixel = { (float)x + 0.5f,(float)y + 0.5f };

		// Edge Equation
		// 使用整数避免浮点误差，同时因为是左手系，所以符号取反
		int E01 = -(x - p[0].x_) * (p[1].y_ - p[0].y_) + (y - p[0].y_) * (p[1].x_ - p[0].x_);
		int E12 = -(x - p[1].x_) * (p[2].y_ - p[1].y_) + (y - p[1].y_) * (p[2].x_ - p[1].x_);
		int E20 = -(x - p[2].x_) * (p[0].y_ - p[2].y_) + (y - p[2].y_) * (p[0].x_ - p[2].x_);


		// 如果是左上边，用 E >= 0 判断合法，如果右下边就用 E > 0 判断合法
		// 这里通过引入一个误差 1 ，来将 < 0 和 <= 0 用一个式子表达
		if (E01 < (TopLeft01 ? 0 : 1)) continue;   // 在第一条边后面
		if (E12 < (TopLeft12 ? 0 : 1)) continue;   // 在第二条边后面
		if (E20 < (TopLeft20 ? 0 : 1)) continue;   // 在第三条边后面
	

		// 三个端点到当前点的矢量
		TinyMath::Vec2f s0 = vs[0]->center_ - pixel;
		TinyMath::Vec2f s1 = vs[1]->center_ - pixel;
		TinyMath::Vec2f s2 = vs[2]->center_ - pixel;

		// 重心坐标系：计算内部子三角形面积 a / b / c
		float a = std::abs(s1.Cross(s2));    // 子三角形 Px-P1-P2 面积
		float b = std::abs(s2.Cross(s0));    // 子三角形 Px-P2-P0 面积
		float c = std::abs(s0.Cross(s1));    // 子三角形 Px-P0-P1 面积
		float s = a + b + c;                 // 大三角形 P0-P1-P2 面积

		if (s == 0.0f) continue;

		// Barycentric coordinates interpolation
		a = a * (1.0f / s);
		b = b * (1.0f / s);
		c = c * (1.0f / s);

		//深度以及其倒数在可以直接使用重心插值
		float rhw = vs[0]->rhw_ * a + vs[1]->rhw_ * b + vs[2]->rhw_ * c;

		//depth_buffer数组下标和x,y相反
		if (rhw < m_depth_buffer[y][x])continue;
		m_depth_buffer[y][x] = rhw;
		
		float w = 1.0f / ((rhw != 0.0f) ? rhw : 1.0f);

		//透视正确插值
		float c0 = vs[0]->rhw_ * a * w;
		float c1 = vs[1]->rhw_ * b * w;
		float c2 = vs[2]->rhw_ * c * w;

		ShaderContext input;
		ShaderContext& i0 = vs[0]->context_;
		ShaderContext& i1 = vs[1]->context_;
		ShaderContext& i2 = vs[2]->context_;


		// 插值各项 varying
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
	if (m_render_line)
	{
		m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
		m_canvas->DrawLine(vs[0]->coords_.x_, vs[0]->coords_.y_, vs[2]->coords_.x_, vs[2]->coords_.y_, m_color_fg);
		m_canvas->DrawLine(vs[2]->coords_.x_, vs[2]->coords_.y_, vs[1]->coords_.x_, vs[1]->coords_.y_, m_color_fg);
	}

}
