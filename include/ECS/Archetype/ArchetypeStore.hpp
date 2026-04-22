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

        struct EntityLocation {
            Archetype * archetype = nullptr;
            Archetype::EntityLocation location{};
        };

        static const Signature kEmptySignature;

        ArrayPool<MAX_COMPONENTS, 4> recordsPool;
        std::unordered_map<Signature, std::unique_ptr<Archetype> > archetypes;
        const std::shared_ptr<ComponentRegistry> registry;
        const std::unique_ptr<ArchetypeStoreChangeNotifier> changeNotifier;
        const std::unique_ptr<ArchetypeFactory> factory;

        std::array<EntityLocation, MAX_ENTITIES> entitiesMap{};

        Archetype *getOrCreateArchetype(const Signature &bitmask) noexcept {
            if (bitmask.none()) {
                return nullptr;
            }
            auto it = archetypes.find(bitmask);
            if (it != archetypes.end()) {
                return it->second.get();
            }
            auto archetype = factory->createArchetypeDynamic(bitmask);
            archetype->entityAddressesSubscription.emplace_back([&](Entity entity, Archetype *arch, Archetype::EntityLocation&& location){
                auto& address = entitiesMap[entity];
                address.archetype = arch;
                address.location = location;
            });
            auto [newIt, _] = archetypes.emplace(bitmask, std::move(archetype));
            auto ptr = newIt->second.get();
            changeNotifier->notifyAdd(ptr);
            return ptr;
        }

        void registerComponents(std::span<const ComponentInfo> &infos) const noexcept {
            for (const auto &info: infos) { registry->registerComponent(info.type); }
        }

        template<typename... Components>
        void fillComponentsRecord(std::span<void *> record, Components &&... components) noexcept {
            ((registry->registerComponent(ComponentTypeID::getTypeInfo<Components>()),
              record[ComponentTypeID::get<Components>()] = &components), ...);
        }

        static void migrateEntity(
            const Entity entity,
            Archetype *prevArchetype,
            Archetype *nextArchetype,
            const std::span<void *> record
        ) {
            nextArchetype->set(record);
            if (prevArchetype != nullptr && prevArchetype != nextArchetype) {
                prevArchetype->remove(entity);
            }
        }

    public:
        ArchetypeStore() : registry(std::make_shared<ComponentRegistry>()), changeNotifier(std::make_unique<ArchetypeStoreChangeNotifier>()),
                           factory(std::make_unique<ArchetypeFactory>(registry)) {
            registry->registerComponent(ComponentTypeID::getTypeInfo<Entity>());
        }

        [[nodiscard]] const std::unique_ptr<ArchetypeStoreChangeNotifier> &getChangeNotifier() const { return changeNotifier; }

        [[nodiscard]] std::vector<const Archetype *> findArchetypes(const Signature &signature, const Signature &excluding) const noexcept {
            std::vector<const Archetype *> results;
            results.reserve(archetypes.size());
            for (const auto &[bitset, archetype]: archetypes) {
                if ((bitset.bitset & signature.bitset) == signature.bitset && (bitset.bitset & excluding.bitset) == 0) {
                    const Archetype *archetypePtr = archetype.get();
                    results.push_back(archetypePtr);
                }
            }
            return results;
        }

        template<typename... Components>
        bool setComponents(Entity entity, Components &&... components) {
            static const auto componentsBitmask = SignatureID<Components...>::signature();
            auto *prevArchetype = entitiesMap[entity].archetype;
            Signature bitmask = componentsBitmask;
            auto record = ComponentsRecord{};
            record[0] = &entity;
            if (prevArchetype) {
                bitmask |= prevArchetype->getSignature();
                if (!prevArchetype->fillComponentRecord(record)) {
                    return false;
                }
            }
            fillComponentsRecord(record, std::forward<Components>(components)...);
            const auto nextArchetype = getOrCreateArchetype(bitmask);
            if (!nextArchetype) {
                return false;
            }
            migrateEntity(entity, prevArchetype, nextArchetype, record);
            if (prevArchetype != nullptr && prevArchetype != nextArchetype) {
                changeNotifier->notifyUpdate(prevArchetype);
            }
            changeNotifier->notifyUpdate(nextArchetype);
            return true;
        }

        template<typename Component>
        bool removeComponent(Entity entity) {
            static const auto removed = ComponentTypeID::getTypeInfo<Component>();
            auto prevArchetype = entitiesMap[entity].archetype;
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
                entitiesMap[entity].archetype = nullptr;
                return true;
            }
            auto nextArchetype = getOrCreateArchetype(bitmask);
            if (!nextArchetype) {
                prevArchetype->remove(entity);
                changeNotifier->notifyUpdate(prevArchetype);
                entitiesMap[entity].archetype = nullptr;
                return false;
            }
            auto record = ComponentsRecord{};
            record[0] = &entity;
            if (!prevArchetype->fillComponentRecord(record)) {
                return false;
            }
            migrateEntity(entity, prevArchetype, nextArchetype, record);
            // entitiesMap[entity].archetype = nextArchetype;
            // entitiesMap[entity].location = *nextArchetype->getEntityLocation(entity);
            changeNotifier->notifyUpdate(prevArchetype);
            changeNotifier->notifyUpdate(nextArchetype);
            return true;
        }

        bool removeEntity(const Entity entity) noexcept {
            const auto archetype = entitiesMap[entity].archetype;
#ifndef NDEBUG
            assert(archetype != nullptr);
#endif
            if (!archetype) {
                return false;
            }
            archetype->remove(entity);
            changeNotifier->notifyUpdate(archetype);
            entitiesMap[entity].archetype = nullptr;
            return true;
        }

        template<typename Component>
        [[nodiscard]] bool hasComponent(const Entity entity) const noexcept {
            const auto* archetype = entitiesMap[entity].archetype;
            if (!archetype) {
                return false;
            }
            const auto type = ComponentTypeID::get<Component>();
            return archetype->getSignature().test(type);
        }

        [[nodiscard]] const Signature& getSignature(const Entity entity) const noexcept {
            const auto* archetype = entitiesMap[entity].archetype;
            if (!archetype) {
                return kEmptySignature;
            }
            return archetype->getSignature();
        }

        template<typename Component>
        [[nodiscard]] inline Component *getComponent(const Entity entity) const noexcept {
            static const auto typeId = ComponentTypeID::get<Component>();
            const auto& location = entitiesMap[entity];
            const auto* archetype = location.archetype;
#ifndef NDEBUG
            assert(archetype != nullptr);
#endif
            if (!archetype) {
                [[unlikely]] return nullptr;
            }
            if (!archetype->getSignature().test(typeId)) {
                [[unlikely]] return nullptr;
            }
            return reinterpret_cast<Component *>(archetype->getComponentByLocation(location.location, typeId));
        }

        inline bool fillComponentRecord(Entity entity, std::span<void *> record) const {
            const auto& location = entitiesMap[entity];
            record[0] = &entity;
            return location.archetype->fillComponentRecordByLocation(record, location.location);
        }

        template<size_t N>
        inline bool fillComponentsInRecord(Entity entity, std::span<void *> record, const std::array<const ComponentType, N> components) const {
            const auto& location = entitiesMap[entity];
            record[0] = &entity;
            return location.archetype->fillComponentRecordByTypesByLocation<N>(record, location.location, components);
        }

        template<typename Component>
        static ComponentTypeIndex getTypeIndex() noexcept {
            return ComponentTypeID::get<Component>();
        }
    };
}
