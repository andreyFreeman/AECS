//
// Created by ANDREY KLADOV on 23/05/2025.
//

#pragma once

#include "ComponentView.hpp"
#include "ECS/Archetype/ArchetypeStore.hpp"

namespace ECS {
    template<typename... Components>
    class ComponentViewSubscribed final {
        const Signature including = SignatureID<Components...>::signature();
        const std::unique_ptr<ArchetypeStore> &store;
        const Signature excluding;

        std::unique_ptr<ComponentView<Components...> > view;
        bool reloadData = true;

        size_t subscriptionIdUpdate;
        size_t subscriptionIdAdd;

        const std::unique_ptr<ComponentView<Components...>>& getView() {
            if (!view) {
                view = std::make_unique<ComponentView<Components...> >(store->findArchetypes(including, excluding));
            } else if (reloadData) {
                reloadData = false;
                store->fillArchetypesMatching(including, excluding, view->getArchetypes());
                view->updateIterationData();
            }
            return view;
        }

    public:
        explicit ComponentViewSubscribed(const std::unique_ptr<ArchetypeStore> &store, const Signature &excluding) : store(store), excluding(excluding) {
            subscriptionIdUpdate = store->getChangeNotifier()->subscribeToUpdate([this](const auto *archetype) {
                if (reloadData) return;
                const auto &signature = archetype->getSignature().bitset;
                reloadData = (including.bitset & signature) == including.bitset && (this->excluding.bitset & signature).none();
            });
            subscriptionIdAdd = store->getChangeNotifier()->subscribeToAdd([this](const auto *archetype) {
                if (reloadData) return;
                const auto &signature = archetype->getSignature().bitset;
                reloadData = (including.bitset & signature) == including.bitset && (this->excluding.bitset & signature).none();
            });
        }

        ComponentIterator<Components...> begin() {
            return getView()->begin();
        }

        ComponentIterator<Components...> end() {
            return getView()->end();
        }

        template<typename Func>
        requires std::invocable<Func, Components&...>
        bool forEach(Func&& func) { return getView()->forEach(std::forward<Func>(func)); }

        ~ComponentViewSubscribed() {
            store->getChangeNotifier()->unsubscribeFromUpdate(subscriptionIdUpdate);
            store->getChangeNotifier()->unsubscribeFromAdd(subscriptionIdAdd);
        }
    };
}
