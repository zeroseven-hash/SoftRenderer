#pragma once
#include"Renderer/Renderer.h"

#include<vector>
#include<iostream>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>


struct TextureCompnent
{
    uint32_t id_;
    shared_ptr<Texture2D> texture_;
    std::string type_;
    std::string path_;
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

    template<typename SHADER>
    void Draw(SHADER& shader)
    {
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            std::string& name = m_textures[i].type_;
            if (name == "texture_diffuse") shader.SetTexture("u_diffuse", 0, m_textures[i].texture_.get());
           // m_textures[i].SaveFileBMP("texture.bmp");
        }
        Renderer::Submit(m_vao, shader);
    }
private:
    VertexArrayBuffer<V> m_vao;
    std::vector<TextureCompnent> m_textures;
};

struct aiNode;
struct aiScene;
struct aiMesh;
template<typename V>
class Model
{
public:
    Model() = default;
    Model(const char* path)
    {
        LoadModel(path);
    }

public:
    void LoadModel(const char* path);

    template<typename SHADER>
    void Draw(SHADER& shader)
    {
        for (size_t i = 0; i < m_meshes.size(); i++) m_meshes[i].Draw(shader);
    }
private:
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh<V> ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<TextureCompnent> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& type_name);
private:
    std::vector<Mesh<V>> m_meshes;
    std::string m_dir;
};


template<typename V>
void Model<V>::LoadModel(const char* path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::string s_path(path);
    m_dir = s_path.substr(0, s_path.find_last_of('/'));
    ProcessNode(scene->mRootNode, scene);
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
        vertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<TextureCompnent> diffuse_maps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
    // std::vector<Texture> specular_maps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    // textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
    // std::vector<Texture> normal_maps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    // textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
    // std::vector<Texture> height_maps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    // textures.insert(textures.end(), height_maps.begin(), height_maps.end());

    return Mesh<V>(std::move(vertices), std::move(indices), std::move(textures));
}
template<typename V>
std::vector<TextureCompnent> Model<V>::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& type_name)
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
        texture.texture_ = std::make_shared<Texture2D>(TextureLayout::LINEAR);
        texture.texture_->LoadFile(filename.c_str(),SAMPLER_NEARST|SAMPLER_REPEAT);
        texture.id_ = texturescount++;
        texture.type_ = type_name;
        textures.push_back(texture);
    }
    return textures;
}
