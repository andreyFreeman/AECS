//
//  ComponentTypeID.hpp
//  swipe_puzzle
//
//  Created by ANDREY KLADOV on 14/05/2025.
//

#pragma once

#include <atomic>
#include "ComponentTypeInfo.hpp"
#include <Templates.hpp>

namespace ECS {
    class ComponentTypeID final {

        static inline std::atomic<uint32_t> counter{0};

        template<typename T>
        static constexpr ComponentTypeIndex get() {
            using DecayedT = decay<T>;
            static const ComponentTypeIndex id = getID<DecayedT>();
            return id;
        }

        template<typename DecayedT>
        static constexpr ComponentTypeIndex getID() {
            static const uint32_t id = counter.fetch_add(1, std::memory_order_relaxed);
            return id;
        }

        template<typename T>
        static constexpr ComponentTypeInfo getTypeInfo() {
            return {get<T>(), sizeof(T), alignof(T)};
        }

        template<typename... Components>
        static constexpr std::array<ComponentTypeInfo, sizeof...(Components)> getTypeInfos() {
            std::array<ComponentTypeInfo, sizeof...(Components)> typeInfos{ComponentTypeID::getTypeInfo<decay<Components> >()...};
            return typeInfos;
        }

    public:
        friend class ArchetypeFactory;
        friend class Archetype;
        friend class ArchetypeStore;
        template<typename... Components>
        friend class ComponentView;
        template<typename... Components>
        friend class SignatureID;
    };

    template<typename... Components>
    class SignatureID final {
        static constexpr Signature signature() {
            Signature signature;
            (signature.set(ComponentTypeID::get<std::decay_t<Components> >()), ...);
            return signature;
        }

        friend class Archetype;
        friend class ArchetypeStore;
        friend class EntityManager;
        template<typename... C>
        friend class ComponentViewSubscribed;
        template<typename... C>
        friend class ComponentView;
    };
}
