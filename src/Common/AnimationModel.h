#pragma once
#include"Model.h"


const int MAX_BOUNE_INFLUENCE = 4;
struct BoneInfo
{
	int id_;
	aiMatrix4x4 offset_;
};

struct AnimationVertex
{
	TinyMath::Vec4f pos_;
	TinyMath::Vec3f normal_;
	TinyMath::Vec2f coords_;
	TinyMath::Vec3f tangent_;
	TinyMath::Vec3f bitangent_;

	int             bone_ids_[MAX_BOUNE_INFLUENCE];
	float           weights_[MAX_BOUNE_INFLUENCE];
	
};

template<typename V= AnimationVertex>
class AnimationModel :public Model<V>
{
public:

    AnimationModel(const char* filepath):Model<V>()
    {
        LoadModel(filepath);
    }
	Mesh<V> ProcessMesh(aiMesh* mesh, const aiScene* scene)override;

    
	auto& get_bonemap() { return m_bone_info_map; }
	int get_bone_num(){ return m_bone_num; }

private:
    void ExtractBoneWeightForVertices(std::vector<V>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (int bone_index = 0; bone_index < mesh->mNumBones; bone_index++)
        {
            int bone_id = -1;
            std::string bone_name = mesh->mBones[bone_index]->mName.C_Str();
            if (!m_bone_info_map.count(bone_name))
            {
                BoneInfo bone;
                bone.id_ = m_bone_num;
                bone.offset_ = mesh->mBones[bone_index]->mOffsetMatrix;
                m_bone_info_map[bone_name] = bone;
                bone_id = m_bone_num;
                m_bone_num++;
            }
            else
            {
                bone_id = m_bone_info_map[bone_name].id_;
            }
            assert(bone_id != -1);

            auto weights = mesh->mBones[bone_index]->mWeights;
            int wieght_num = mesh->mBones[bone_index]->mNumWeights;

            for (int i = 0; i < wieght_num; i++)
            {
                int v_id = weights[i].mVertexId;
                float weight = weights[i].mWeight;
                assert(v_id <= vertices.size());
                auto& vertex = vertices[v_id];
                for (int j = 0; j < MAX_BOUNE_INFLUENCE; j++)
                {
                    if (vertex.bone_ids_[j] < 0)
                    {
                        vertex.bone_ids_[j] = bone_id;
                        vertex.weights_[j] = weight;
                        break;
                    }
                }
            }
        }
    }
private:
	std::map<std::string, BoneInfo> m_bone_info_map;
	int m_bone_num = 0;
};

template<typename V>
Mesh<V> AnimationModel<V>::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<V> vertices;
    std::vector<uint32_t> indices;
    std::vector<TextureCompnent> textures;


    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        V vertex;
        vertex.pos_ = TinyMath::Vec4f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);

        for (int i = 0; i < MAX_BOUNE_INFLUENCE; i++)
        {
            vertex.bone_ids_[i] = -1;
            vertex.weights_[i] = 0.0f;
        }

        if (mesh->mTextureCoords[0])
        {
            vertex.coords_.x_ = mesh->mTextureCoords[0][i].x;
            vertex.coords_.y_ = mesh->mTextureCoords[0][i].y;
        }
        else
            vertex.coords_ = TinyMath::Vec2f(0.0f, 0.0f);

        if (mesh->HasNormals())
        {
            vertex.normal_.x_ = mesh->mNormals[i].x;
            vertex.normal_.y_ = mesh->mNormals[i].y;
            vertex.normal_.z_ = mesh->mNormals[i].z;
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.bitangent_.x_ = mesh->mBitangents[i].x;
            vertex.bitangent_.y_ = mesh->mBitangents[i].y;
            vertex.bitangent_.z_ = mesh->mBitangents[i].z;


            vertex.tangent_.x_ = mesh->mTangents[i].x;
            vertex.tangent_.y_ = mesh->mTangents[i].y;
            vertex.tangent_.z_ = mesh->mTangents[i].z;
        }
        vertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<TextureCompnent> kd_maps = LoadMaterialTextures(material, aiTextureType_DIFFUSE);
    textures.insert(textures.end(), kd_maps.begin(), kd_maps.end());
    std::vector<TextureCompnent> ks_maps = LoadMaterialTextures(material, aiTextureType_SPECULAR);
    textures.insert(textures.end(), ks_maps.begin(), ks_maps.end());
    std::vector<TextureCompnent> bump_maps = LoadMaterialTextures(material, aiTextureType_HEIGHT);
    textures.insert(textures.end(), bump_maps.begin(), bump_maps.end());
    std::vector<TextureCompnent> ka_maps = LoadMaterialTextures(material, aiTextureType_AMBIENT);
    textures.insert(textures.end(), ka_maps.begin(), ka_maps.end());

    ExtractBoneWeightForVertices(vertices, mesh, scene);

    auto m = Mesh<V>(std::move(vertices), std::move(indices), std::move(textures));
    m.SetAABB(mesh->mAABB);
    return m;
}