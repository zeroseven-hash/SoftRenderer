#include "Bone.h"

void Bone::LoadChannel(const aiNodeAnim* channel)
{
	int translate_count = channel->mNumPositionKeys;
	int rotate_count = channel->mNumRotationKeys;
	int scale_count = channel->mNumScalingKeys;

	m_translates.reserve(translate_count);
	m_rotates.reserve(rotate_count);
	m_scales.reserve(scale_count);
	for (int i = 0; i < translate_count; i++)
	{
		KeyTranslate translate;
		translate.translate_ = channel->mPositionKeys[i].mValue;
		translate.time_stamp_ = (float)channel->mPositionKeys[i].mTime;
		m_translates.emplace_back(translate);
	}

	for (int i = 0; i < rotate_count; i++)
	{
		KeyRotate rotate;
		rotate.rotate_= channel->mRotationKeys[i].mValue;
		rotate.time_stamp_ = (float)channel->mRotationKeys[i].mTime;
		m_rotates.emplace_back(rotate);
	}


	for (int i = 0; i < scale_count; i++)
	{
		KeyScale scale;
		scale.scale_ = channel->mScalingKeys[i].mValue;
		scale.time_stamp_ = (float)channel->mScalingKeys[i].mTime;
		m_scales.emplace_back(scale);
	}
}

aiMatrix4x4 Bone::get_local_transform(float current_time)
{
	aiVector3D translate = InterpolateTranslate(current_time);
	aiQuaternion rotate= InterpolateRotate(current_time);
	aiVector3D scale= InterpolateScale(current_time);

	return aiMatrix4x4(scale, rotate, translate);
}

static float CalcScaleFactor(float a, float b, float t)
{
	float total = b - a;
	float part = t - a;
	return part / total;
}

static aiVector3D LinearInterpolate(const aiVector3D& a, const aiVector3D& b, float t)
{
	aiVector3D res;
	for (int i = 0; i < 3; i++)
	{
		res[i] = a[i] + (b[i] - a[i]) * t;
	}
	return res;
}
aiVector3D Bone::InterpolateTranslate(float current_time)
{
	if (m_translates.size() == 0) return aiVector3D();
	if (m_translates.size() == 1) return m_translates[0].translate_;

	int index = 0;
	for (; index < m_translates.size() - 1; index++)
	{
		if (current_time < m_translates[index+1].time_stamp_)
			break;
	}
	int left = index;
	int right = index + 1;

	float t = CalcScaleFactor(m_translates[left].time_stamp_, m_translates[right].time_stamp_, current_time);
	

	
	return LinearInterpolate(m_translates[left].translate_, m_translates[right].translate_, t);
}

aiQuaternion Bone::InterpolateRotate(float current_time)
{
	if (m_rotates.size() == 0) return aiQuaternion();
	if (m_rotates.size() == 1) return m_rotates[0].rotate_;

	int index = 0;
	for (; index < m_rotates.size() - 1; index++)
	{
		if (current_time < m_rotates[index + 1].time_stamp_)
			break;
	}
	int left = index;
	int right = index + 1;

	float t = CalcScaleFactor(m_rotates[left].time_stamp_, m_rotates[right].time_stamp_, current_time);

	aiQuaternion res;
	aiQuaternion::Interpolate(res, m_rotates[left].rotate_, m_rotates[right].rotate_, t);
	return res;
}

aiVector3D Bone::InterpolateScale(float current_time)
{
	if (m_scales.size() == 0) return aiVector3D();
	if (m_scales.size() == 1) return m_scales[0].scale_;

	int index = 0;
	for (; index < m_scales.size() - 1; index++)
	{
		if (current_time < m_scales[index + 1].time_stamp_)
			break;
	}
	int left = index;
	int right = index + 1;

	float t = CalcScaleFactor(m_scales[left].time_stamp_, m_scales[right].time_stamp_, current_time);



	return LinearInterpolate(m_scales[left].scale_, m_scales[right].scale_, t);
}
