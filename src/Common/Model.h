#pragma once
#include"Renderer/Renderer.h"

#include<vector>
#include<iostream>

#include<assimp/Importer.hpp>
#include<assimp/Exporter.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>



struct TextureCompnent
{
    uint32_t id_;
    shared_ptr<Texture2D> texture_;
    aiTextureType type_;
    std::string path_;
};

struct ModelVertex
{
    TinyMath::Vec4f pos_;
    TinyMath::Vec3f normal_;
    TinyMath::Vec2f coords_;
    TinyMath::Vec3f tangent_;
    TinyMath::Vec3f bitangent_;
};

template<typename V>
class Mesh
{
public:
    Mesh() = default;
    Mesh(const std::vector<V>& vs, const std::vector<uint32_t>& inds, const std::vector<TextureCompnent> texs)
        :m_textures(texs)
    {
        m_vao.set_vertices(vs);
        m_vao.set_indices(inds);
    }
    Mesh(std::vector<V>&& vs, std::vector<uint32_t>&& inds, std::vector<TextureCompnent>&& texs)
        :m_textures(std::move(texs))
    {
        m_vao.set_vertices(std::move(vs));
        m_vao.set_indices(std::move(inds));
    }
public:

    const aiAABB& GetAABB()const  { return m_AABB; }
    void SetAABB(const aiAABB& aabb) { m_AABB = aabb; }
    template<typename SHADER>
    void Draw(SHADER& shader)
    {
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            auto type = m_textures[i].type_;
            if (type == aiTextureType_DIFFUSE)  shader.SetTexture(0, m_textures[i].texture_.get());
            if (type == aiTextureType_SPECULAR) shader.SetTexture(1, m_textures[i].texture_.get());
            if (type == aiTextureType_AMBIENT)  shader.SetTexture(2, m_textures[i].texture_.get());
            if (type == aiTextureType_HEIGHT)   shader.SetTexture(3, m_textures[i].texture_.get());
            if(type  == aiTextureType_EMISSIVE)    shader.SetTexture(4, m_textures[i].texture_.get());

            if(type==aiTextureType_NORMALS) shader.SetTexture(5, m_textures[i].texture_.get());
            if (type == aiTextureType_METALNESS) shader.SetTexture(6, m_textures[i].texture_.get());
            if (type == aiTextureType_LIGHTMAP) shader.SetTexture(7, m_textures[i].texture_.get());
           // m_textures[i].SaveFileBMP("texture.bmp");
        }
        Renderer::Submit(m_vao, shader);
    }
private:
    VertexArrayBuffer<V> m_vao;
    std::vector<TextureCompnent> m_textures;
    aiAABB m_AABB;
};

struct aiNode;
struct aiScene;
struct aiMesh;
template<typename V=ModelVertex>
class Model
{
public:
    Model() = default;
    Model(const char* path)
    {
        LoadModel(path);
    }

public:
    virtual void LoadModel(const char* path);

    const aiAABB& GetAABB()const { return m_AABB; }
    template<typename SHADER>
    void Draw(SHADER& shader)
    {
        for (size_t i = 0; i < m_meshes.size(); i++) m_meshes[i].Draw(shader);
    }
protected:
    void LoadMesh(const aiScene* scene);
    void ProcessNode(aiNode* node, const aiScene* scene);
    virtual Mesh<V> ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureCompnent> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
protected:
    std::vector<Mesh<V>> m_meshes;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_textures_;
    aiAABB m_AABB;
    std::string m_dir;
};


template<typename V>
void Model<V>::LoadModel(const char* path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate  |aiProcess_FlipUVs |aiProcess_CalcTangentSpace|aiProcess_GenBoundingBoxes|aiProcess_LimitBoneWeights);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::string s_path(path);
    m_dir = s_path.substr(0, s_path.find_last_of('/'));
    LoadMesh(scene);
}


template<typename V>
inline void Model<V>::LoadMesh(const aiScene* scene)
{
    ProcessNode(scene->mRootNode, scene);
    m_AABB = m_meshes[0].GetAABB();
    for (auto& mesh : m_meshes)
    {
        const auto& AABB = mesh.GetAABB();
        m_AABB.mMax.x = std::max(AABB.mMax.x, m_AABB.mMax.x);
        m_AABB.mMax.y = std::max(AABB.mMax.y, m_AABB.mMax.y);
        m_AABB.mMax.z = std::max(AABB.mMax.z, m_AABB.mMax.z);

        m_AABB.mMin.x = std::min(AABB.mMin.x, m_AABB.mMin.x);
        m_AABB.mMin.y = std::min(AABB.mMin.y, m_AABB.mMin.y);
        m_AABB.mMin.z = std::min(AABB.mMin.z, m_AABB.mMin.z);
    }
}

template<typename V>
void Model<V>::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(mesh, scene));
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}
template<typename V>
Mesh<V> Model<V>::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<V> vertices;
    std::vector<uint32_t> indices;
    std::vector<TextureCompnent> textures;

    
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        V vertex;
        vertex.pos_ = TinyMath::Vec4f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        
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

    auto m= Mesh<V>(std::move(vertices), std::move(indices), std::move(textures));
    m.SetAABB(mesh->mAABB);
    return m;
}
template<typename V>
std::vector<TextureCompnent> Model<V>::LoadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    static int texturescount = 0;
    std::vector<TextureCompnent> textures;
    for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        TextureCompnent texture;

        std::string filename = std::string(str.C_Str());
        filename = m_dir + '/' + filename;
        if (m_textures_.count(filename))
        {
            texture.texture_ = m_textures_[filename];
        }
        else
        {
            texture.texture_ = std::make_shared<Texture2D>(filename.c_str(), TextureLayout::LINEAR);
            m_textures_[filename] = texture.texture_;
        }

        //texture.texture_->GenerateMipmap();
        texture.id_ = texturescount++;
        texture.type_ = type;
        textures.push_back(texture);
    }
    return textures;
}
