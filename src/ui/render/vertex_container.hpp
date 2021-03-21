//
// Created by Simon Farre: 2021-03-18.
// Github: https://github.com/theIDinside
//

#pragma once
#include <cstddef>

template<typename T>
constexpr auto bytes_size_of(int countOfT) { return sizeof(T) * countOfT; }


template<typename VertexT>
class VertexContainer {
public:
    explicit VertexContainer(std::size_t reservedVertices);

    template <std::size_t ArraySize>
    constexpr void push_quad6(VertexT (&quad)[ArraySize]);
    constexpr void* data();
    constexpr auto bytes_allocated() const;
private:
    VertexT* ptr_data;
    std::size_t m_size;
    std::size_t m_cap;
};
