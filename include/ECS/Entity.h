//
//  Entity.h
//  ECS
//
//  Created by ANDREY KLADOV on 25/05/2024.
//

#pragma once

#include <iostream>
#include "Component/SignatureBitset.hpp"

#ifndef MAX_COMPONENTS
#define MAX_COMPONENTS 128
#endif

#ifndef CHUNK_SIZE
#define CHUNK_SIZE (128 * 1024)
#endif

#ifndef MAX_CHUNK_COUNT
#define MAX_CHUNK_COUNT 1024
#endif

#ifndef MAX_ENTITIES
#define MAX_ENTITIES 10000000
#endif

namespace ECS {

    using Entity = uint64_t;
    using ComponentType = uint16_t;
    using ComponentTypeIndex = ComponentType;
    using Signature = SignatureBitset<MAX_COMPONENTS>;
    using SignatureKey = uint64_t;
    using QueryKey = SignatureKey;

}
