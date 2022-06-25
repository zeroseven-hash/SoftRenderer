#pragma once
#include"Common/Model.h"
#include"Common/Application.h"
#include"Animator.h"
#include"Bone.h"
const int MAX_BOUNE_INFLUENCE = 5;

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

static TinyMath::Mat4f AssimpMatToMyMat(const aiMatrix4x4& mat)
{
    TinyMath::Mat4f res;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            res[i][j] = mat[i][j];
        }
    }
    return res;
}

template<typename V= AnimationVertex>
class AnimationModel :public Model<V>
{
public:

    AnimationModel(const char* filepath):Model<V>()
    {
        LoadModel(filepath);

    }
    void LoadModel(const char* filepath)override;
    
    void UpdateAnime(TimeStep ts);
	auto& get_bonemap() { return m_bone_info_map; }
	int get_bone_num(){ return m_bone_num; }
    std::vector<TinyMath::Mat4f>* get_bone_transform() { return &m_final_tramsform; }
private:
	Mesh<V> ProcessMesh(aiMesh* mesh, const aiScene* scene)override;
    void ReadChannels(const aiAnimation* animation);
    void LoadAnimation(const aiScene* scene);
    void ReadHeirarchyData(AssimpNodeData& node, const aiNode* src, const aiMatrix4x4& parienttramsform);
    //wait to be optimzied!
    void UpdateFinalMatrix(const AssimpNodeData& node, const aiMatrix4x4& parieent_transform);
    void ExtractBoneWeightForVertices(std::vector<V>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (uint32_t bone_index = 0; bone_index < mesh->mNumBones; bone_index++)
        {
            int bone_id = -1;
            std::string bone_name = mesh->mBones[bone_index]->mName.C_Str();
            if (!m_bone_info_map.count(bone_name))
            {
                
                int id = m_bone_num;
                auto& offset = mesh->mBones[bone_index]->mOffsetMatrix;
                m_bone_info_map[bone_name] = m_bone_num;
                m_bones.emplace_back(id,bone_name,offset);
                bone_id = m_bone_num;
                m_bone_num++;
            }
            else
            {
                bone_id = m_bone_info_map[bone_name];
            }
            assert(bone_id != -1);

            auto weights = mesh->mBones[bone_index]->mWeights;
            int wieght_num = mesh->mBones[bone_index]->mNumWeights;

            for (int i = 0; i < wieght_num; i++)
            {
                int v_id = weights[i].mVertexId;
                float weight = weights[i].mWeight;
                assert(v_id < vertices.size());
                auto& vertex = vertices[v_id];
                for (int j = 0; j < MAX_BOUNE_INFLUENCE; j++)
                {
                    if (vertex.bone_ids_[j] == -1)
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
	std::map<std::string, int> m_bone_info_map;
    std::vector<Bone> m_bones;
    std::vector<TinyMath::Mat4f> m_final_tramsform;


	int m_bone_num = 0;
    float m_duration;
    float m_ticks_per_second;
    float m_current_time=0.0f;
    bool m_has_animation = false;


    AssimpNodeData m_RootNode;
    aiMatrix4x4 m_global_transform_inverse;
};

template<typename V>
inline void AnimationModel<V>::LoadModel(const char* filepath)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::string s_path(filepath);
    m_dir = s_path.substr(0, s_path.find_last_of('/'));
    LoadMesh(scene);
    LoadAnimation(scene);
}

template<typename V>
inline void AnimationModel<V>::UpdateAnime(TimeStep ts)
{
    if (!m_has_animation) return;
    m_current_time += ts.get_second() * m_ticks_per_second;
    m_current_time = std::fmod(m_current_time, m_duration);
    UpdateFinalMatrix(m_RootNode,aiMatrix4x4());
}




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
    std::vector<TextureCompnent> emissive_maps = LoadMaterialTextures(material, aiTextureType_EMISSIVE);
    textures.insert(textures.end(), emissive_maps.begin(), emissive_maps.end());

    std::vector<TextureCompnent> normal_maps = LoadMaterialTextures(material,aiTextureType_NORMALS);
    textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

    std::vector<TextureCompnent> mellic_rough_maps = LoadMaterialTextures(material, aiTextureType_METALNESS);
    textures.insert(textures.end(), mellic_rough_maps.begin(), mellic_rough_maps.end());
    
    
    std::vector<TextureCompnent> ao_maps = LoadMaterialTextures(material,aiTextureType_LIGHTMAP);
    textures.insert(textures.end(), ao_maps.begin(), ao_maps.end());

    ExtractBoneWeightForVertices(vertices, mesh, scene);

   
    auto m = Mesh<V>(std::move(vertices), std::move(indices), std::move(textures)); 
  
    m.SetAABB(mesh->mAABB);
    return m;
}



template<typename V>
inline void AnimationModel<V>::LoadAnimation(const aiScene* scene)
{
    if (scene->HasAnimations()) m_has_animation = true;
    else
    {
        m_has_animation = false;
        return;
    }
    auto animation = scene->mAnimations[0];
    m_duration = (float)animation->mDuration;
    m_ticks_per_second = (float)animation->mTicksPerSecond;
    m_global_transform_inverse = scene->mRootNode->mTransformation;
    m_global_transform_inverse.Inverse();
    m_final_tramsform.resize(m_bones.size(),Mat4f::GetIdentity());

    ReadChannels(animation);
    ReadHeirarchyData(m_RootNode, scene->mRootNode, aiMatrix4x4());
}

template<typename V>
inline void AnimationModel<V>::ReadHeirarchyData(AssimpNodeData& node, const aiNode* src,const aiMatrix4x4& parienttramsform)
{
    node.name_ = src->mName.data;
    node.transformation_ = src->mTransformation;
    node.children_count_ = src->mNumChildren;

    aiMatrix4x4 global_matrix = parienttramsform * node.transformation_;
    if (m_bone_info_map.count(node.name_))
    {
        auto& bone = m_bones[m_bone_info_map[node.name_]];
        m_final_tramsform[bone.get_id()] = AssimpMatToMyMat(global_matrix * bone.get_offset());
    }
   

    for (uint32_t i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData new_node;
        ReadHeirarchyData(new_node, src->mChildren[i],global_matrix);
        node.children_.push_back(new_node);
    }
}

template<typename V>
inline void AnimationModel<V>::UpdateFinalMatrix(const AssimpNodeData& node, const aiMatrix4x4& parient_transform)
{
    const auto& name = node.name_;
    const aiMatrix4x4& node_transform = node.transformation_;

    aiMatrix4x4 global_transform;

    if (m_bone_info_map.count(node.name_))
    {

        auto& bone = m_bones[m_bone_info_map[node.name_]];
        if (!bone.m_rotates.size() && !bone.m_translates.size() && !bone.m_scales.size())
        {
            global_transform = parient_transform * node.transformation_;
        }
        else
            global_transform = parient_transform * bone.get_local_transform(m_current_time);
        
        if(bone.get_id()<m_final_tramsform.size()) m_final_tramsform[bone.get_id()] = AssimpMatToMyMat(global_transform * bone.get_offset());

    }
    else
        global_transform = parient_transform * node.transformation_;

    for (int i = 0; i < node.children_count_; i++)
    {
        UpdateFinalMatrix(node.children_[i], global_transform);
    }
}

template<typename V>
inline void AnimationModel<V>::ReadChannels(const aiAnimation* animation)
{
    int size = animation->mNumChannels;

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string bone_name = channel->mNodeName.data;
        //std::cout << bone_name << "\n";

        if (m_bone_info_map.count(bone_name))
        {
            int bone_id = m_bone_info_map[bone_name];
            m_bones[bone_id].LoadChannel(channel);
        }
       
        /*else
        {
            m_bone_info_map[bone_name] = m_bone_num;
            m_bones.emplace_back(m_bone_num, bone_name, aiMatrix4x4());
            m_bone_num++;
        }*/
        
    }

}
