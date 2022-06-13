#pragma once
#include<vector>
#include"assimp/Importer.hpp"
struct AssimpNodeData
{
	aiMatrix4x4 transformation_;
	std::string name_;
	int children_count_;
	std::vector<AssimpNodeData> children_;
};
//class Animation
//{
//	Animation() = default;
//};