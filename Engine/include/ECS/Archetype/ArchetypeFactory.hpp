//
//  ArchetypeFactory.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 19/05/2025.
//

#pragma once

#include "Archetype.hpp"
#include "Chunks/ChunkFactory.hpp"
#include <ECS/Component/ComponentTypeInfo.hpp>

namespace ECS {
    class ArchetypeFactory final {
        const std::shared_ptr<ComponentRegistry> registry;

    public:
        ArchetypeFactory(const std::shared_ptr<ComponentRegistry>& registry): registry(registry) { }

        __attribute__((noinline)) std::unique_ptr<Archetype> createArchetypeDynamic(const Signature &signature) {
            return std::make_unique<Archetype>(registry, signature, std::make_unique<ChunkFactory>(signature, registry, CHUNK_SIZE, MAX_CHUNK_COUNT));
        }
    };
}
