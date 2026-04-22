//
//  EntityManager.hpp
//  ECS
//
//  Created by ANDREY KLADOV on 29/05/2024.
//

#pragma once

#include "Archetype/ArchetypeStore.hpp"
#include "Archetype/ComponentView/ComponentViewSubscribed.hpp"
#include "Entity.h"

namespace ECS {
    class EntityManager final {
        std::vector<Entity> deleted;
        Entity lastCreated = 0;

        std::unique_ptr<ArchetypeStore> archetypeStore;

        Entity getIndex() {
            if (!deleted.empty()) {
                const auto value = deleted.back();
                deleted.pop_back();
                return value;
            }
            return ++lastCreated;
        }

    public:
        EntityManager() : archetypeStore(std::make_unique<ArchetypeStore>()) {
        }

        Entity create() {
            auto entity = getIndex();
            archetypeStore->setComponents(entity, entity);
            return entity;
        }

        template<typename... Components>
        Entity createWithComponents(Components &&... components) {
            auto value = getIndex();
            if (!archetypeStore->setComponents(value, value, std::forward<Components>(components)...)) {
#ifndef NDEBUG
                throw std::runtime_error("Failed to setup components");
#endif
            }
            return value;
        }

        template<typename Component>
        inline Component *getComponent(const Entity entity) const { return archetypeStore->getComponent<Component>(entity); }

        inline bool fillComponentRecord(const Entity entity, const std::span<void *> record) const {
            return archetypeStore->fillComponentRecord(entity, record);
        }

        template<size_t N>
        inline bool fillComponentsInRecord(const Entity entity, const std::span<void *> record, const std::array<const ComponentType, N> components) const {
            return archetypeStore->fillComponentsInRecord<N>(entity, record, components);
        }

        template<typename Component>
        void setComponent(Entity entity, Component &&component) {
            if (!archetypeStore->setComponents(entity, std::forward<Component>(component))) {
#ifndef NDEBUG
                throw std::runtime_error("Failed to setup component");
#endif
            }
        }

        template<typename... Components>
        void setComponents(const Entity entity, Components &&... components) {
            if (!archetypeStore->setComponents(entity, std::forward<Components>(components)...)) {
#ifndef NDEBUG
                throw std::runtime_error("Failed to setup components");
#endif
            }
        }

        template<typename Component>
        [[nodiscard]] bool hasComponent(const Entity entity) const { return archetypeStore->hasComponent<Component>(entity); }

        [[nodiscard]] const Signature& getSignature(const Entity entity) const  { return archetypeStore->getSignature(entity); }

        template<typename Component>
        void removeComponent(const Entity entity) const {
            if (!hasComponent<Component>(entity)) {
                return;
            }
            archetypeStore->removeComponent<Component>(entity);
        }

        void remove(const Entity entity) {
            archetypeStore->removeEntity(entity);
            deleted.push_back(entity);
        }

        template<typename... Components>
        struct Query{};
        
        template<typename... Included, typename... Excluded>
        [[nodiscard]]
        ComponentViewSubscribed<Included...> createComponentViewWithQuery(Query<Included...>, Query<Excluded...>) const {
            Signature excluding = SignatureID<Excluded...>::signature();
            return ComponentViewSubscribed<Included...>(archetypeStore, excluding);
        }
        
        template<typename... Components>
        [[nodiscard]] ComponentViewSubscribed<Components...> createComponentView() const {
            return createComponentViewWithQuery(Query<Components...>{}, Query{});
        }
    };
}
