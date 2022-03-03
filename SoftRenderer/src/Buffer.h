#pragma once
#include<vector>
typedef unsigned int uint32_t;

template<typename T, typename U = uint32_t>
class VertexArrayBuffer
{
public:
    VertexArrayBuffer() = default;
    VertexArrayBuffer(const std::vector<T>& vs, const std::vector<U>& indices) :m_vertices(vs), m_indices(indices) {}
    VertexArrayBuffer(std::vector<T>&& vs, std::vector<U>&& indices) :m_vertices(std::move(vs)), m_indices(std::move(indices)) {}
    ~VertexArrayBuffer() = default;

public:
    const std::vector<U>& get_indices()const { return m_indices; }
    const std::vector<T>& get_vertices()const { return m_vertices; }
    const T& get_vertex(size_t i)const { assert(i < m_vertices.size());return m_vertices[i]; }

    void set_indices(const std::vector<U>& indices) { m_indices = indices; }
    void set_indices(std::vector<U>&& indices) { m_indices = std::move(indices); }

    void set_vertices(const std::vector<T>& vertices) { m_vertices = vertices; }
    void set_vertices(std::vector<T>&& vertices) { m_vertices = std::move(vertices); }
private:
    std::vector<T> m_vertices;
    std::vector<U> m_indices;
};