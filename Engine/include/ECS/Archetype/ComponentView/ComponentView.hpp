//
//  ArchetypeComponentView.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 20/05/2025.
//

#pragma once

#include <ECS/Archetype/Archetype.hpp>
#include "ComponentIterator.hpp"
#include <ECS/Component/ComponentTypeID.hpp>
#include <Templates.hpp>
#include <memory>
#include <thread>

namespace ECS {
    template<typename... Components>
    class ForwardPointerReader final {
        static const size_t N = sizeof...(Components);

        std::array<char *, N> currentPtrs;
        std::array<size_t, N> strides;

        const char *chunkEnd;
        size_t remainingEntities;

        const std::vector<IterationMeta<N> > &chunks;
        size_t chunkIdx;

        void advanceToNextChunk() {
            if (++chunkIdx >= chunks.size()) {
                remainingEntities = 0;
                return;
            }

            const auto &meta = chunks[chunkIdx];
            for (size_t i = 0; i < N; ++i) {
                currentPtrs[i] = meta.ptrs[i];
                strides[i] = meta.stride[i];
            }

            chunkEnd = currentPtrs[0] + meta.count * strides[0];
            remainingEntities = meta.count;
        }

        template<typename Func, std::size_t... Is>
        void callFunc(Func &&func, std::index_sequence<Is...>) {
            func(*reinterpret_cast<Components *>(currentPtrs[Is])...);
        }

    public:
        explicit ForwardPointerReader(const std::vector<IterationMeta<N> > &chunks): chunks(chunks), chunkIdx(0), remainingEntities(0) {
            if (!chunks.empty()) {
                const auto &meta = chunks[0];
                for (size_t i = 0; i < N; ++i) {
                    currentPtrs[i] = meta.ptrs[i];
                    strides[i] = meta.stride[i];
                }
                chunkEnd = currentPtrs[0] + meta.count * strides[0];
                remainingEntities = meta.count;
            }
        }

        template<typename Func>
        void forEach(Func &&func) {
            while (remainingEntities > 0) {
                callFunc(std::forward<Func>(func), std::make_index_sequence<N>{});
                for (auto i = 0; i < N; ++i) {
                    currentPtrs[i] += strides[i];
                }
                if (--remainingEntities == 0) {
                    advanceToNextChunk();
                }
            }
        }

        template<typename Func>
        void forEachParallel(Func &&func) {
            while (remainingEntities > 0) {
                std::array<char *, N> chunkPtrs = currentPtrs;
                const size_t chunkSize = remainingEntities;
                for (size_t entity = 0; entity < chunkSize; ++entity) {
                    func(*reinterpret_cast<Components *>(chunkPtrs[0])...);
                    for (size_t i = 0; i < N; ++i) {
                        chunkPtrs[i] += strides[i];
                    }
                }
                remainingEntities = 0;
                advanceToNextChunk();
            }
        }

        [[nodiscard]] bool hasMore() const {
            return remainingEntities > 0;
        }
    };

    template<typename... Components>
    class ComponentView final {
        static const size_t N = sizeof...(Components);
        Signature signature = SignatureID<Components...>::signature();
        std::vector<const Archetype *> archetypes;
        std::vector<IterationMeta<N> > iterationData;

        void updateIterationData() {
            iterationData.clear();
            for (auto i = 0; i < archetypes.size(); ++i) {
                const auto &archetype = archetypes[i];
                for (const Chunks::Chunk &chunk: archetype->getChunks()) {
                    if (chunk.size == 0) {
                        continue;
                    }
                    IterationMeta<N> meta;
                    meta.count = chunk.size;
                    auto j = 0;
                    for (auto type = signature.lowestBit; type <= signature.highestBit; ++type) {
                        if (signature[type]) {
                            const auto &component = chunk.components[type];
                            meta.ptrs[j] = component.ptr;
                            meta.stride[j] = component.stride;
                            ++j;
                        }
                    }
                    iterationData.push_back(meta);
                }
            }
        }

        void addArchetype(const Archetype *archetype) {
            archetypes.push_back(archetype);
        }

        template<typename Func, std::size_t... Is>
        void invoke(Func &&func, const IterationMeta<N> &meta, size_t index, std::index_sequence<Is...>) const {
            func(*reinterpret_cast<Components *>(meta.ptrs[Is] + index * meta.stride[Is])...);
        }

    public:
        explicit ComponentView(const std::vector<const Archetype *> &archetypes) {
            for (const auto &archetype: archetypes) {
                addArchetype(archetype);
            }
            updateIterationData();
        }

        template<typename Func>
        void forEach(Func &&func) const {
            auto reader = ForwardPointerReader<Components...>(iterationData);
            reader.forEach(std::forward<Func>(func));
        }

        ComponentIterator<Components...> begin() const { return ComponentIterator<Components...>(iterationData); }
        ComponentIterator<Components...> end() const { return ComponentIterator<Components...>(iterationData, true); }
    };
}
