//
// Created by ANDREY KLADOV on 26/05/2025.
//

#pragma once

#include <array>
#include <stack>

template <size_t MaxComponents, size_t PoolSize = 128>
class ArrayPool {
public:
    using ArrayType = std::array<const void*, MaxComponents>;

    ArrayPool() {
        index_ = PoolSize;
    }

    ArrayType* acquire() {
        return &storage_[--index_];
    }

    void release(ArrayType* buffer) {
        storage_[index_++] = *buffer;
    }

private:
    std::array<ArrayType, PoolSize> storage_;
    size_t index_;
};
