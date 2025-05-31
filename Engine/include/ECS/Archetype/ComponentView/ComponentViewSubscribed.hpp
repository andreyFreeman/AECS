//
// Created by ANDREY KLADOV on 23/05/2025.
//

#pragma once

#include "ComponentView.hpp"
#include "ECS/Archetype/ArchetypeStore.hpp"

namespace ECS {
    template<typename... Components>
    class ComponentViewSubscribed final {
        const Signature bitmask = SignatureID<Components...>::signature();
        const std::unique_ptr<ArchetypeStore> &store;

        std::unique_ptr<ComponentView<Components...> > view;
        bool reloadData = true;

        const std::unique_ptr<ComponentView<Components...>>& getView() {
            if (reloadData || !view) {
                reloadData = false;
                view = std::make_unique<ComponentView<Components...> >(store->findArchetypes(bitmask));
            }
            return view;
        }

    public:
        explicit ComponentViewSubscribed(const std::unique_ptr<ArchetypeStore> &store) : store(store) {
            store->getChangeNotifier()->subscribeToUpdate([this](const auto archetype) {
                const auto &signature = archetype->getSignature();
                reloadData = (bitmask.bitset & signature.bitset) == signature.bitset;
            });
            store->getChangeNotifier()->subscribeToAdd([this](const auto archetype) {
                const auto &signature = archetype->getSignature();
                reloadData = (bitmask.bitset & signature.bitset) == signature.bitset;
            });
        }

        ComponentIterator<Components...> begin() {
            return getView()->begin();
        }

        ComponentIterator<Components...> end() {
            return getView()->end();
        }

        template<typename Func>
        void forEach(Func&& func) { getView()->forEach(func); }
    };
}
