#pragma once
#include<vector>

#include"assimp/Importer.hpp"
#include"assimp/anim.h"
struct KeyTranslate
{
	aiVector3D translate_;
	float time_stamp_;
};

struct KeyRotate
{
	aiQuaternion rotate_;
	float time_stamp_;
};
struct KeyScale
{
	aiVector3D scale_;
	float time_stamp_;
};


class Bone
{
public:
	Bone(int id, const std::string& name, aiMatrix4x4 offset)
		:m_id(id),m_name(name),m_offset(offset){}

	void LoadChannel(const aiNodeAnim* channel);
public:
	aiMatrix4x4 get_local_transform(float current_time);
	int get_id()const { return m_id; }
	const aiMatrix4x4& get_offset() const { return m_offset; }

private:
	aiVector3D InterpolateTranslate(float current_time);
	aiQuaternion InterpolateRotate(float current_time);
	aiVector3D InterpolateScale(float current_time);

public:
	aiMatrix4x4 m_offset;
	aiMatrix4x4 m_local_transform;

	std::vector<KeyTranslate> m_translates;
	std::vector<KeyRotate>	m_rotates;
	std::vector<KeyScale>	m_scales;

	std::string m_name;
	int  m_id;
};