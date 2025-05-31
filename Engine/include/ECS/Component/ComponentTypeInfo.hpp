//
//  ComponentTypeInfo.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 18/05/2025.
//

#pragma once

#include <ECS/Entity.h>

namespace ECS {
    struct ComponentTypeInfo {
        ComponentType type;
        uint16_t size;
        uint16_t alignment;
    };

    struct ComponentInfo {
        const ComponentTypeInfo type;
        const char *component;
    };
}
