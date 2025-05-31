//
//  ArchetypeIterator.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 20/05/2025.
//

#pragma once

#include "IterationMeta.hpp"

namespace ECS {
    template<typename... Components>
    class ComponentIterator final {
    public:
        using iterator_category = std::forward_iterator_tag;
        using TupleRef = std::tuple<Components &...>;

    private:
        static constexpr size_t N = sizeof...(Components);

        std::array<char *, N> currentPtrs;
        std::array<size_t, N> strides;

        const char *chunkEnd;
        size_t remainingEntities;

        const std::vector<IterationMeta<N> > &chunks;
        size_t chunk_idx;

        void advanceToNextChunk() {
            if (++chunk_idx >= chunks.size()) {
                remainingEntities = 0;
                return;
            }

            const auto &meta = chunks[chunk_idx];
            for (size_t i = 0; i < N; ++i) {
                currentPtrs[i] = meta.ptrs[i];
                strides[i] = meta.stride[i];
            }

            chunkEnd = currentPtrs[0] + meta.count * strides[0];
            remainingEntities = meta.count;
        }

        template<std::size_t... Is>
        TupleRef makeTupleImpl(std::index_sequence<Is...>) const {
            return std::tie(*reinterpret_cast<Components *>(currentPtrs[Is])...);
        }

    public:
        explicit ComponentIterator(const std::vector<IterationMeta<N> > &data, bool end = false)
            : chunks(data), chunk_idx(end ? data.size() : 0), remainingEntities(0) {
            if (!end && !data.empty()) {
                chunk_idx = 0;
                const auto &meta = data[0];

                for (size_t i = 0; i < N; ++i) {
                    currentPtrs[i] = meta.ptrs[i];
                    strides[i] = meta.stride[i];
                }

                chunkEnd = currentPtrs[0] + meta.count * strides[0];
                remainingEntities = meta.count;
            }
        }

        bool operator!=(const ComponentIterator &other) const {
            return remainingEntities != other.remainingEntities || chunk_idx != other.chunk_idx;
        }

        TupleRef operator*() const {
            return makeTupleImpl(std::make_index_sequence<N>{});
        }

        ComponentIterator &operator++() {
            for (size_t i = 0; i < N; ++i) {
                currentPtrs[i] += strides[i];
            }
            if (--remainingEntities == 0) {
                advanceToNextChunk();
            }
            return *this;
        }

        void prefetchNext(int distance = 1) const {
            if (remainingEntities > distance) {
                __builtin_prefetch(currentPtrs[0] + distance * strides[0], 0, 3);
            }
        }
    };
}
