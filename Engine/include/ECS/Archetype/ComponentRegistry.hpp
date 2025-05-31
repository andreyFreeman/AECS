//
// Created by ANDREY KLADOV on 25/05/2025.
//

#pragma once

#include <array>
#include "ECS/Component/ComponentTypeInfo.hpp"

namespace ECS {
    class ComponentRegistry final {
        std::array<ComponentTypeInfo, MAX_COMPONENTS> components{};
        std::bitset<MAX_COMPONENTS> bitset{};
    public:
        __attribute__((noinline)) void registerComponent(const ComponentTypeInfo type) {
            if (bitset.test(type.type)) {
                return;
            }
            components[type.type] = type;
            bitset.set(type.type);
        }

        [[nodiscard]] ComponentTypeInfo getType(const ComponentType type) const {
            return components[type];
        }

        [[nodiscard]] bool isRegistered(const ComponentType type) const {
            return bitset.test(type);
        }

        [[nodiscard]] const ComponentTypeInfo operator[](const ComponentType type) const {
            return components[type];
        }

        [[nodiscard]] ComponentTypeInfo operator[](const ComponentType type) {
            return components[type];
        }

        [[nodiscard]] std::vector<ComponentTypeInfo> getTypes(const Signature& bitset) const {
            std::vector<ComponentTypeInfo> types;
            types.reserve(bitset.highestBit - bitset.lowestBit + 1);
            for (size_t i = bitset.lowestBit; i <= bitset.highestBit; ++i) {
                if (bitset[i]) {
                    types.push_back(components[i]);
                }
            }
            return types;
        }
    };
}

