#pragma once
#include<vector>
#include<memory>
typedef unsigned int uint32_t;
using std::shared_ptr;



template<typename T>
class VertexArrayBuffer
{
public:
    using VertexType = T;
    VertexArrayBuffer() = default;
    VertexArrayBuffer(const std::vector<T>& vs, const std::vector<uint32_t>& indices) :m_vertices(vs), m_indices(indices) {}
    VertexArrayBuffer(std::vector<T>&& vs, std::vector<uint32_t>&& indices) :m_vertices(std::move(vs)), m_indices(std::move(indices)) {}
    ~VertexArrayBuffer() = default;

public:
    const std::vector<uint32_t>& get_indices()const { return m_indices; }
    const std::vector<T>& get_vertices()const { return m_vertices; }
    const T& get_vertex(size_t i)const { assert(i < m_vertices.size()); return m_vertices[i]; }

    void set_indices(const std::vector<uint32_t>& indices) { m_indices = indices; }
    void set_indices(std::vector<uint32_t>&& indices) { m_indices = std::move(indices); }

    void set_vertices(const std::vector<T>& vertices) { m_vertices = vertices; }
    void set_vertices(std::vector<T>&& vertices) { m_vertices = std::move(vertices); }

    static shared_ptr<VertexArrayBuffer<T>> Create(const std::vector<T>& vs, const std::vector<uint32_t>& indices)
    {
        return std::make_shared<VertexArrayBuffer<T>>(vs, indices);
    }

    static shared_ptr<VertexArrayBuffer<T>> Create(std::vector<T>&& vs, std::vector<uint32_t>&& indices)
    {
        return std::make_shared<VertexArrayBuffer<T>>(std::move(vs),std::move(indices));
    }
private:
    std::vector<T> m_vertices;
    std::vector<uint32_t> m_indices;
};