#include"Shader.h"





TinyMath::Vec4f ToLinear(const TinyMath::Vec4f& color)
{
	auto res = TinyMath::Pow(color,2.2f);
	res.a_ = color.a_;
	return res;
}

TinyMath::Vec4f ToGammar(const TinyMath::Vec4f& color)
{
	const float gamma = 1.0f / 2.2f;

	auto res = TinyMath::Pow(color, gamma);
	res.a_ = color.a_;
	return res;
}

TinyMath::Vec3f ToGammar(const TinyMath::Vec3f& color)
{
	const float gamma = 1.0f / 2.2f;
	return TinyMath::Pow(color, gamma);

}

TinyMath::Vec3f Exposure(const TinyMath::Vec3f& color, float exposure)
{
	auto res=TinyMath::Vec3f(1.0f, 1.0f, 1.0f) - TinyMath::Exp(-color * exposure);
	return res;
}
