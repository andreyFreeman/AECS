//
//  ArchetypeStore.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 17/05/2025.
//

#pragma once

#include <memory>
#include <utility>
#include <ECS/Entity.h>
#include <ECS/Component/ComponentTypeID.hpp>
#include "Archetype.hpp"
#include "ArchetypeFactory.hpp"
#include "ArchetypeStoreChangeNotifier.hpp"
#include "ArrayPool.hpp"
#include "ComponentRegistry.hpp"

namespace ECS {
    class ArchetypeStore final {
        static constexpr auto kEmptyComponents = std::array<ComponentTypeInfo, 0>();

        ArrayPool<MAX_COMPONENTS, 4> recordsPool;
        std::unordered_map<Signature, std::unique_ptr<Archetype> > archetypes;
        std::unordered_map<Entity, Archetype *> entities;
        const std::shared_ptr<ComponentRegistry> registry;
        const std::unique_ptr<ArchetypeStoreChangeNotifier> changeNotifier;
        const std::unique_ptr<ArchetypeFactory> factory;

        Archetype *getOrCreateArchetype(const Signature &bitmask) noexcept {
            if (bitmask.none()) {
                return nullptr;
            }
            auto it = archetypes.find(bitmask);
            if (it != archetypes.end()) {
                return it->second.get();
            }
            auto archetype = factory->createArchetypeDynamic(bitmask);
            auto [newIt, _] = archetypes.emplace(bitmask, std::move(archetype));
            auto ptr = newIt->second.get();
            changeNotifier->notifyAdd(ptr);
            return ptr;
        }

        void registerComponents(const std::span<const ComponentInfo> &infos) noexcept {
            for (const auto &info: infos) { registry->registerComponent(info.type); }
        }

        template<typename... Components>
        void fillComponentsRecord(std::span<const void*> record, const Components &... components) noexcept {
            ((registry->registerComponent(ComponentTypeID::getTypeInfo<Components>()),
              record[ComponentTypeID::get<Components>()] = &components), ...);
        }

        static void migrateEntity(
            const Entity entity,
            Archetype *prevArchetype,
            Archetype *nextArchetype,
            std::span<const void *> record
        ) {
            nextArchetype->set(record);
            if (prevArchetype != nullptr && prevArchetype != nextArchetype) {
                prevArchetype->remove(entity);
            }
        }

    public:
        ArchetypeStore() : registry(std::make_shared<ComponentRegistry>()), changeNotifier(std::make_unique<ArchetypeStoreChangeNotifier>()), factory(std::make_unique<ArchetypeFactory>(registry)) {
            registry->registerComponent(ComponentTypeID::getTypeInfo<Entity>());
        }

        [[nodiscard]] const std::unique_ptr<ArchetypeStoreChangeNotifier> &getChangeNotifier() const { return changeNotifier; }

        std::vector<const Archetype *> findArchetypes(const Signature &signature) const noexcept {
            std::vector<const Archetype *> results;
            results.reserve(archetypes.size());
            for (const auto &[bitset, archetype]: archetypes) {
                if ((bitset.bitset & signature.bitset) == signature.bitset) {
                    const Archetype *archetypePtr = archetype.get();
                    results.push_back(archetypePtr);
                }
            }
            return results;
        }

        template<typename... Components>
        bool setComponents(const Entity entity, const Components &... components) {
            static const auto componentsBitmask = SignatureID<Components...>::signature();
            auto it = entities.find(entity);
            const auto prevArchetype = it != entities.end() ? it->second : nullptr;
            Signature bitmask = componentsBitmask;
            auto record = ComponentsRecord{};
            record[0] = &entity;
            if (prevArchetype) {
                bitmask |= prevArchetype->getSignature();
                if (!prevArchetype->fillComponentRecord(record)) {
                    return false;
                }
            }
            fillComponentsRecord(record, components...);
            const auto nextArchetype = getOrCreateArchetype(bitmask);
            if (!nextArchetype) {
                return false;
            }
            migrateEntity(entity, prevArchetype, nextArchetype, record);
            if (it != entities.end()) {
                it->second = nextArchetype;
            } else {
                entities.emplace(entity, nextArchetype);
            }
            if (prevArchetype != nullptr && prevArchetype != nextArchetype) {
                changeNotifier->notifyUpdate(prevArchetype);
            }
            changeNotifier->notifyUpdate(nextArchetype);
            return true;
        }

        template<typename Component>
        bool removeComponent(const Entity entity) {
            static const auto removed = ComponentTypeID::getTypeInfo<Component>();
            auto it = entities.find(entity);
            auto prevArchetype = it != entities.end() ? it->second : nullptr;
            if (!prevArchetype) {
                return false;
            }
            auto bitmask = prevArchetype->getSignature();
            if (!bitmask.test(removed.type)) {
                return false;
            }
            bitmask.reset(removed.type);
            if (bitmask.none()) {
                prevArchetype->remove(entity);
                changeNotifier->notifyUpdate(prevArchetype);
                entities.erase(entity);
                return true;
            }
            auto nextArchetype = getOrCreateArchetype(bitmask);
            if (!nextArchetype) {
                prevArchetype->remove(entity);
                changeNotifier->notifyUpdate(prevArchetype);
                entities.erase(entity);
                return false;
            }
            auto record = ComponentsRecord{};
            record[0] = &entity;
            if (!prevArchetype->fillComponentRecord(record)) {
                return false;
            }
            migrateEntity(entity, prevArchetype, nextArchetype, record);
            if (it != entities.end()) {
                it->second = nextArchetype;
            } else {
                entities.emplace(entity, nextArchetype);
            }
            changeNotifier->notifyUpdate(prevArchetype);
            changeNotifier->notifyUpdate(nextArchetype);
            return true;
        }

        bool removeEntity(const Entity entity) noexcept {
            auto it = entities.find(entity);
            if (it == entities.end()) {
                return false;
            }
            const auto archetype = it->second;
            if (!archetype) {
                return false;
            }
            archetype->remove(entity);
            changeNotifier->notifyUpdate(archetype);
            entities.erase(it);
            return true;
        }

        template<typename Component>
        [[nodiscard]] bool hasComponent(const Entity entity) const noexcept {
            const auto it = entities.find(entity);
            if (it == entities.end()) {
                return false;
            }
            const auto archetype = it->second;
            if (!archetype) {
                return false;
            }
            const auto type = ComponentTypeID::get<Component>();
            return archetype->getSignature().test(type);
        }

        template<typename Component>
        [[nodiscard]] Component *getComponent(const Entity entity) const noexcept {
            static const auto typeId = ComponentTypeID::get<Component>();
            const auto it = entities.find(entity);
            if (it == entities.end()) {
                return nullptr;
            }
            const auto archetype = it->second;
            if (!archetype) {
                return nullptr;
            }
            const auto type = ComponentTypeID::get<Component>();
            if (!archetype->getSignature().test(type)) {
                return nullptr;
            }
            return static_cast<Component *>(archetype->getComponent(entity, typeId));
        }
    };
}
