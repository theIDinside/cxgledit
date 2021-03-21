//
// Created by Simon Farre: 2021-03-18.
// Github: https://github.com/theIDinside
//

#include "vertex_container.hpp"
#include <memory>
#include <cstring>

template<typename VertexT>
VertexContainer<VertexT>::VertexContainer(std::size_t reservedVertices) : ptr_data(new VertexT[reservedVertices]), m_size(0), m_cap(reservedVertices) {

}

template<typename VertexT>
constexpr auto VertexContainer<VertexT>::bytes_allocated() const {
    return sizeof(VertexT) * m_cap;
}

template<typename VertexT>
template<std::size_t ArraySize>
constexpr void VertexContainer<VertexT>::push_quad6(VertexT (&quad)[ArraySize]) {
    static_assert(ArraySize == 6, "Quad is represented currently as 2 triangles, thus 6 vertices. Wasteful, but it's how it works for now");
    if(m_cap - 6 < m_size) { // reallocate buffer

    }
    std::memcpy(ptr_data + m_size, &quad, bytes_size_of<VertexT>(ArraySize));
    m_size += 6;
}

template<typename VertexT>
constexpr void* VertexContainer<VertexT>::data() {
    return ptr_data;
}
