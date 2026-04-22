//
//  Updatable.hpp
//  arcade_ninja
//
//  Created by ANDREY KLADOV on 23/04/2025.
//

#pragma once

class Updatable {
public:
    virtual bool update(float dt) = 0;
    virtual ~Updatable() = default;
};

struct UpdatableFn {
    void* context;
    bool (*fn)(void*, float);
};

template <typename T>
UpdatableFn makeUpdatable(T* instance) {
    return {instance, [](void* ctx, float dt) {
        return static_cast<T*>(ctx)->update(dt);
    }};
}

inline bool callUpdate(const UpdatableFn& u, const float dt) {
    return u.fn(u.context, dt);
}
