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
        Entity createWithComponents(const Components &... components) {
            const auto value = getIndex();
            if (!archetypeStore->setComponents(value, value, components...)) {
#ifndef NDEBUG
                throw std::runtime_error("Failed to setup components");
#endif
            }
            return value;
        }

        template<typename Component>
        Component *getComponent(const Entity entity) const { return archetypeStore->getComponent<Component>(entity); }

        template<typename Component>
        void setComponent(const Entity entity, const Component &component) {
            if (!archetypeStore->setComponents(entity, component)) {
#ifndef NDEBUG
                throw std::runtime_error("Failed to setup component");
#endif
            }
        }

        template<typename... Components>
        void setComponents(const Entity entity, const Components &... components) {
            if (!archetypeStore->setComponents(entity, entity, components...)) {
#ifndef NDEBUG
                throw std::runtime_error("Failed to setup components");
#endif
            }
        }

        template<typename Component>
        [[nodiscard]] bool hasComponent(const Entity entity) const { return archetypeStore->hasComponent<Component>(entity); }

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
        [[nodiscard]] ComponentViewSubscribed<Components...> createComponentView() const {
            return ComponentViewSubscribed<Components...>(archetypeStore);
        }
    };
}
