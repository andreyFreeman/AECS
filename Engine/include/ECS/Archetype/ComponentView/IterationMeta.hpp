//
//  IterationMeta.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 23/05/2025.
//

#pragma once

namespace ECS {
    template <size_t N>
    struct IterationMeta {
        char *ptrs[N];
        size_t stride[N];
        size_t count;
    };
}
