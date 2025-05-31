//
//  SystemComponentView.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 13/05/2025.
//

#pragma once

#include "System.hpp"
#include <ECS/Archetype/ComponentView/ComponentViewSubscribed.hpp>

namespace ECS {
    template<typename... Components>
    class SystemComponentView : public Updatable {

    protected:
        ComponentViewSubscribed<Components...> componentView;

    public:
        using EntityInfo = std::tuple<Components &...>;

        explicit SystemComponentView(ComponentViewSubscribed<Components...> &&componentView) : componentView(std::move(componentView)) {
        }

        explicit SystemComponentView(const EntityManager &manager) : componentView(manager.createComponentView<Components...>()) {
        }
    };
}
