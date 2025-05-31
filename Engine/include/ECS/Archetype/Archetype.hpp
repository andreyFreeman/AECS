//
//  ArchetypeNG.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 18/05/2025.
//

#pragma once

#include "Chunks/ChunkFactory.hpp"
#include <algorithm>
#include <ranges>
#include "ComponentRegistry.hpp"

namespace ECS {

    using ComponentsRecord = std::array<const void *, MAX_COMPONENTS>;

    class Archetype final {

        struct EntityLocation {
            Chunks::Index chunkIndex{};
            size_t indexInChunk{};
        };

        const Signature signature;
        const std::shared_ptr<ComponentRegistry> registry;
        const std::unique_ptr<ChunkFactory> chunkFactory;

        size_t count;

        std::vector<Chunks::Chunk> chunks;
        std::vector<std::optional<EntityLocation> > entityLocations;

        bool addChunk() {
            if (const auto chunk = chunkFactory->create()) {
                chunks.push_back(chunk.value());
                return true;
            }
            return false;
        }

        void setEntityLocation(const Entity entity, EntityLocation &&location) noexcept {
            if (entity >= entityLocations.size()) {
                entityLocations.resize(entity + 1);
            }
            entityLocations[entity] = std::forward<EntityLocation>(location);
        }

        void removeEntityLocation(const Entity entity) {
            if (entity < entityLocations.size()) {
                entityLocations[entity] = std::nullopt;
            } else {
#ifndef NDEBUG
                throw std::runtime_error("removeEntityLocation: entity out of range");
#endif
            }
        }

        [[nodiscard]] std::optional<EntityLocation> getEntityLocation(const Entity entity) const {
            if (entity < entityLocations.size()) {
                return entityLocations[entity];
            }
            return std::nullopt;
        }

        bool addEntity(const Entity entity, std::span<const void *> data) noexcept {
#ifndef NDEBUG
            assert(entity == *static_cast<const Entity *>(data[0]));
#endif
            Chunks::Index index = chunks.size() - 1;
            for (size_t i = 0; i < chunks.size(); ++i) {
                Chunks::Chunk &chunk = chunks[index];
                if (chunk.size < chunk.capacity) {
                    Chunks::set(chunk, data, chunk.size);
                    setEntityLocation(entity, {index, chunk.size});
                    ++chunk.size;
                    ++count;
                    return true;
                }
                --index;
            }

            if (!addChunk()) {
                return false;
            }

            Chunks::Index last = chunks.size() - 1;
            Chunks::Chunk &chunk = chunks[last];
            Chunks::set(chunk, data, chunk.size);
            setEntityLocation(entity, {last, chunk.size});
            ++chunk.size;
            ++count;
            return true;
        }

    public:

        Archetype(const std::shared_ptr<ComponentRegistry> &registry, const Signature& signature, std::unique_ptr<ChunkFactory> chunkFactory)
            : signature(signature), registry(registry), chunkFactory(std::move(chunkFactory)), count(0) /*, indices(makeIndices(registry, signature))*/ {
            chunks.reserve(this->chunkFactory->getChunkCount());
            entityLocations.reserve(this->chunkFactory->getChunkCount() * this->chunkFactory->getChunkCapacity());
        }

        [[nodiscard]] const Signature &getSignature() const noexcept { return signature; }
        [[nodiscard]] size_t size() const noexcept { return count; }
        [[nodiscard]] uint16_t chunkCount() const noexcept { return chunks.size(); }
        [[nodiscard]] bool empty() const noexcept { return chunks.empty(); }
        [[nodiscard]] const std::vector<Chunks::Chunk> &getChunks() const noexcept { return chunks; }
        [[nodiscard]] const Chunks::Chunk &getChunk(const uint8_t index) const noexcept { return chunks[index]; }
        [[nodiscard]] const Chunks::Chunk &operator[](const std::size_t index) const noexcept { return chunks[index]; }

        bool remove(const Entity entity) {
            const auto location = getEntityLocation(entity);
            if (!location.has_value()) {
                return false;
            }
            const auto &loc = location.value();
            auto &chunk = chunks[loc.chunkIndex];
#ifndef NDEBUG
            if (chunk.size == 0) {
                throw std::runtime_error("Attempt to remove from an empty chunk");
            }
#endif
            const size_t lastIndex = chunk.size - 1;
            if (loc.indexInChunk != lastIndex) {
                const Entity lastEntity = *static_cast<const Entity *>(Chunks::get(chunk, 0, lastIndex));
                Chunks::swap(chunk, loc.indexInChunk, lastIndex);
                setEntityLocation(lastEntity, {loc.chunkIndex, loc.indexInChunk});
            }
            removeEntityLocation(entity);
            --chunk.size;
            --count;
            return true;
        }

        [[nodiscard]] ComponentsRecord getComponentRecord(const Entity entity) const {
            ComponentsRecord record{};
            record[0] = &entity;
#ifndef NDEBUG
            if (!fillComponentRecord(record)) {
                throw std::runtime_error("Failed to fill component record");
            }
#endif
            return record;
        }

        bool fillComponentRecord(std::span<const void*> record) const {
            const auto entity = *static_cast<const Entity *>(record[0]);
            const auto location = getEntityLocation(entity);
            if (!location.has_value()) {
                return false;
            }
            const auto &[chunkIndex, index] = location.value();
            const auto &chunk = chunks[chunkIndex];
            for (auto i = signature.lowestBit; i <= signature.highestBit; i++) {
                if (signature[i]) {
                    record[i] =  Chunks::get(chunk, i, index);
                }
            }
            return true;
        }

        [[nodiscard]] void *getComponent(const Entity entity, const ComponentType type) const noexcept {
            const auto location = getEntityLocation(entity);
            if (!location.has_value()) {
                return nullptr;
            }
            const auto &[chunkIndex, index] = location.value();
            const auto &chunk = chunks[chunkIndex];
            return Chunks::get(chunk, type, index);
        }

        bool set(const std::span<const void *> data) noexcept {
            const auto entity = *static_cast<const Entity *>(data[0]);
            const auto location = getEntityLocation(entity);
            if (!location.has_value()) {
                return addEntity(entity, data);
            }
            const auto &[chunkIndex, index] = location.value();
            const auto &chunk = chunks[chunkIndex];
            Chunks::set(chunk, data, index);
            return true;
        }
    };
}
