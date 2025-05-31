//
//  ChunkFactoryNG.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 18/05/2025.
//

#pragma once

#include <ECS/Component/ComponentTypeInfo.hpp>
#include "Chunk.hpp"
#include "ECS/Archetype/ComponentRegistry.hpp"

namespace ECS {

class ChunkFactory final {
    
    struct ChunkComponentLayout {
        size_t offset;
        uint16_t size;
        uint16_t alignment;
    };

    const Signature bitset;
    const std::shared_ptr<ComponentRegistry> registry;
    const Chunks::Index chunkSize;
    std::array<ChunkComponentLayout, MAX_COMPONENTS> chunkLayout;
    Chunks::Index chunkCapacity = 0;

    char *basePtr = nullptr;
    Chunks::Index chunkCount = 0;
    Chunks::Index chunkCreated = 0;
    
    friend class ArchetypeFactory;

    std::array<Chunks::ComponentData, MAX_COMPONENTS> makeComponents() const {
        std::array<Chunks::ComponentData, MAX_COMPONENTS> components{};
        auto chunkPtr = basePtr + chunkCreated * CHUNK_SIZE;
        for (auto i = bitset.lowestBit; i <= bitset.highestBit; i++) {
            if (bitset[i]) {
                const auto& layout = chunkLayout[i];
                components[i] = {chunkPtr + layout.offset, layout.size};
            }
        }
        return components;
    }

  public:
    
    explicit ChunkFactory(const Signature& bitset, const std::shared_ptr<ComponentRegistry>& registry, size_t chunkSize, size_t reserveChunks)
        : bitset(bitset), registry(registry), chunkSize(chunkSize), chunkCount(reserveChunks) {
        basePtr = static_cast<char *>(std::malloc(reserveChunks * chunkSize));
        if (!basePtr) {
            throw std::bad_alloc();
        }

        size_t entitySize = 0;
        for (auto i = bitset.lowestBit; i <= bitset.highestBit; i++) {
            const auto type = registry->getType(i);
            if (bitset[i]) {
                entitySize += type.size;
            }
        }
        size_t capacity = chunkSize / entitySize;
        std::array<ChunkComponentLayout, MAX_COMPONENTS> layouts{};
        while (capacity > 0) {
            size_t offset = 0;
            bool fits = true;
            for (auto i = bitset.lowestBit; i <= bitset.highestBit; i++) {
                if (bitset[i]) {
                    auto& layout = layouts[i];
                    const auto type = registry->getType(i);
                    if (type.size == 0 || type.alignment == 0 || (type.alignment & (type.alignment - 1)) != 0) {
#ifndef NDEBUG
                        throw std::runtime_error("Invalid component type");
#endif
                    }
                    offset = (offset + type.alignment - 1) & ~(type.alignment - 1);
                    layout.offset = offset;
                    layout.size = type.size;
                    layout.alignment = type.alignment;
                    size_t end = offset + type.size * capacity;
                    if (end > chunkSize) {
                        fits = false;
                        break;
                    }
                    offset = end;
                }
            }
            if (fits)
                break;
#ifndef NDEBUG
            std::cout << "Failed to fit chunk, trying again" << std::endl;
#endif
            --capacity;
        }
        chunkCapacity = capacity;
        chunkLayout = layouts;
    }
    
    ~ChunkFactory() { std::free(basePtr); }

    [[nodiscard]] size_t getChunkCapacity() const { return chunkCapacity; }
    [[nodiscard]] size_t getChunkCount() const { return chunkCount; }
    [[nodiscard]] bool canCreateChunk() const { return chunkCreated < chunkCount; }

    std::optional<Chunks::Chunk> create() {
        if (!canCreateChunk()) {
            return std::nullopt;
        }
        Chunks::Chunk chunk{makeComponents(), 0, chunkCapacity, bitset};
        ++chunkCreated;
        return chunk;
    }
};

}
