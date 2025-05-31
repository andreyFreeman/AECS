//
//  Chunk.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 13/05/2025.
//

#pragma once

#include <vector>
#include <span>

namespace ECS::Chunks {

    using Index = size_t;
    using ComponentIndex = ComponentType;

    struct ComponentData {
        char * ptr;
        uint16_t stride;
    };

    struct Chunk {
        const std::array<ComponentData, MAX_COMPONENTS> components;
        size_t size;
        const Index capacity;
        const Signature signature;
    };

    static bool set(const Chunk &chunk, std::span<const void *> data, Index index) {
        if (index >= chunk.capacity) {
            return false;
        }
        for (auto i = chunk.signature.lowestBit; i <= chunk.signature.highestBit; i++) {
            if (chunk.signature[i]) {
                auto& component = chunk.components[i];
                void *dst = component.ptr + index * component.stride;
                const void *src = data[i];
                std::memcpy(dst, src, component.stride);
            }
        }
        return true;
    }

    static bool add(const Chunk &chunk,  std::span<const void *> data) {
        return set(chunk, data, chunk.size);
    }

    static void swap(const Chunk &chunk, Index source, Index destination) {
        static const int16_t kBufferSize = 512;
        if (source == destination)
            return;

        for (auto i = chunk.signature.lowestBit; i <= chunk.signature.highestBit; i++) {
            if (chunk.signature[i]) {
                const auto &component = chunk.components[i];
                void *ptrA = component.ptr + source * component.stride;
                void *ptrB = component.ptr + destination * component.stride;
                uint8_t temp[kBufferSize];
                std::memcpy(temp, ptrA, component.stride);
                std::memcpy(ptrA, ptrB, component.stride);
                std::memcpy(ptrB, temp, component.stride);
            }
        }
    }

    static void *get(const Chunk &chunk, ComponentIndex componentIndex, Index index) {
        if (index >= chunk.size || componentIndex >= MAX_COMPONENTS) {
            return nullptr;
        }
        const auto &component = chunk.components[componentIndex];
        return component.ptr + index * component.stride;
    }
}
