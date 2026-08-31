//
//  main.cpp
//  ecs_runner
//
//  Created by ANDREY KLADOV on 13/05/2025.
//

#include <iostream>
#include <iomanip>
#include <ECS/EntityManager.hpp>
#include <ECS/System/SystemComponentView.hpp>
#include <simd/simd.h>
#include "World.hpp"

struct A {
    float x{0.0F};
    float y{0.0F};
};

struct B {
    float x{1.0F};
    float y{1.0F};
};

struct C {
    char character{' '};
};

struct D {
    int32_t atk{0};
    int32_t def{0};
};

struct ComponentA {
    bool value;
    float x{0.0F};
    float y{0.0F};
};

struct ComponentB {
    int value;
    float x{1.0F};
    float y{1.0F};
};

struct ComponentC {
    float value;
    char character{' '};
};

struct ComponentD {
    double value;
};

struct ComponentE {
    float value1;
    int value2;
    bool value3;
};

struct ComponentF {
};

struct ComponentG {
};

#ifndef MAX_NAME_LENGTH
#define MAX_NAME_LENGTH 32
#endif

namespace components {
struct NameTag {
    char name[MAX_NAME_LENGTH];
};
}

namespace components {
struct TransformData {
    simd::float3 position{};
    simd::quatf rotation;
    simd::float3 scale{1, 1, 1};
};

struct Transform {
    TransformData transform;
    simd::float4x4 worldMatrix;
    bool isDirty = true;
};

struct TransformChild {
    ECS::Entity parent = ECS::INVALID_ENTITY;
    ECS::Entity nextSibling = ECS::INVALID_ENTITY;
    ECS::Entity prevSibling = ECS::INVALID_ENTITY;
};

struct TransformParent {
    std::size_t childrenCount = 0;
    ECS::Entity first = ECS::INVALID_ENTITY;
    ECS::Entity last = ECS::INVALID_ENTITY;
};

struct Static {};
}

enum Layers : uint32_t {
    None = 0,
    Player = 1 << 0,
    Geometry = 1 << 1,
    NPC = 1 << 2,
    Interactable = 1 << 3,
};

namespace components {
struct Layer {
    Layers value;
};
}

#ifndef MAX_ACTIVE_COLLISIONS
#define MAX_ACTIVE_COLLISIONS 8
#endif

using CollisionMask = uint32_t;

enum class ColliderType : uint8_t {
    Box,
    Sphere
};

namespace components {
struct CollisionData {
    simd::float3 normal;
    simd::float3 point;
    ECS::Entity entity;
    Layers layer;
};

struct Collider {
    ColliderType type;
    simd::float3 data;
    simd::float3 aabbMin;
    simd::float3 aabbMax;
    bool isDirty;
};

struct Collisions {
    CollisionMask collidesWith;
    CollisionData collisions[MAX_ACTIVE_COLLISIONS];
    uint16_t collisionCount;
};
}

enum AbilityID : uint8_t {
    Dash = 1 << 1,
    Shot = 1 << 2,
    Hook = 3 << 3
};

using AbilitiesMask = uint16_t;

namespace components {
struct Abilities {
    AbilityID active;
    AbilitiesMask available;
};

struct AbilityConfigDash {
    float maxDistance = 100.0f;
    float speed = 100.0f;
};

struct AbilityDashState {
    simd::float3 startPosition;
    simd::float3 direction;
    float distanceCovered;
    bool isActive;
};
}

namespace components {
struct PreviousPosition {
    simd::float3 position;
};
}

namespace components {
struct Pawn {

    enum class State : uint8_t {
        Idle,
        Moving
    };

    State state;
};
}

namespace components {
struct Input {
    simd::float2 movementDirection;
    bool hasActiveMovementInput;
    bool abilitySwitch;
};
}

namespace components {
struct RigidBody {
    float mass;
    simd::float3 velocity;
};
}

namespace components {
struct Disabled {};
}


class SystemABEF final : public ECS::SystemComponentView<const ComponentA, ComponentB, ComponentE, const ComponentD> {
    using SystemComponentView::SystemComponentView;

public:
    int counter = 0;

    bool update(float dt) override {
        const auto start = std::chrono::high_resolution_clock::now();
        const auto updated = componentView.forEach([&](const ComponentA &a, ComponentB &b, ComponentE &e, const ComponentD &d) {
            if (a.value) {
                b.value += 1;
                e.value1 += 10;
            }
            counter++;
            return true;
        });
        const auto enumerated = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
        std::cout << "[" << enumerated.count() << "][ns][" << counter << "][" << static_cast<double>(enumerated.count()) / counter << "]" <<
                std::endl;
        counter = 0;
        return updated;
    }
};

static constexpr auto entitiesToAdd = 3300000;

static size_t g_allocationCount = 0;

int main() {
    auto counter = 0;
    static float sink = 0;
    const auto entityManager = std::make_shared<ECS::EntityManager>();

    auto crashTestEntity = entityManager->createWithComponents(
        components::NameTag{.name = "P1"},
        components::Transform{
            .transform = {
                .position = {3.0f, 0.0f, 5.0f}
            }
        },
        components::Layer{.value = Player},
        components::Collider{.type = ColliderType::Sphere, .data = {0.5f, 0, 0}},
        components::Collisions{.collidesWith = Geometry | Interactable | NPC},
        components::Abilities{.active = Dash, .available = Dash | Shot | Hook},
        components::PreviousPosition{},
        components::Pawn{},
        components::AbilityConfigDash{},
        components::AbilityDashState{},
        components::Input{},
        components::RigidBody{}
    );
    entityManager->setComponent(crashTestEntity, components::Disabled{});
    entityManager->removeComponent<components::Disabled>(crashTestEntity);

    auto simdView = entityManager->createComponentView<components::Transform>();
    float finalX = 0.0f;
    simdView.forEach([&finalX](components::Transform& t) {
        t.transform.position = t.transform.position + simd_make_float3(0.5f, 0.5f, 0.5f);
        finalX = t.transform.position.x;
        return true;
    });

    auto view = entityManager->createComponentView<ComponentA, ComponentB, ComponentC>();

    auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 1; i <= entitiesToAdd; i++) {
        entityManager->createWithComponents<ComponentA, ComponentB, ComponentC>({i % 2 == 0}, {i}, {static_cast<float>(i * .1)});
        counter++;
    }
    auto elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][Created][" << counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    for (auto i = 1; i <= entitiesToAdd; i++) {
        if (i % 2 == 0) {
            counter++;
            entityManager->removeComponent<ComponentB>(i);
        }
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][Removed][" << counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < entitiesToAdd; i++) {
        if (i % 2 == 0) {
            entityManager->setComponent<ComponentF>(i, ComponentF());
        } else {
            entityManager->setComponents<ComponentD, ComponentE>(i, ComponentD{10.0 * i}, ComponentE{static_cast<float>(10.0) * i, 10 * i, false});
        }
        counter++;
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][Added][" << counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    auto viewAC = entityManager->createComponentView<ECS::Entity, ComponentA, ComponentC>();
    start = std::chrono::high_resolution_clock::now();
    for (const auto [e, a, c]: viewAC) {
        counter++;
        if (a.value) {
            sink += c.value;
        }
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated AC][" << counter <<
            "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    viewAC.forEach([&counter](auto e, auto a, auto c) {
        counter++;
        if (a.value) {
            sink += c.value - e;
        }
        return true;
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated ForEach AC][" <<
            counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    auto viewA = entityManager->createComponentView<ComponentA>();
    start = std::chrono::high_resolution_clock::now();
    for (const auto [a]: viewA) {
        if (a.value) {
            sink += 2;
        }
        counter++;
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated A][" << counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    viewA.forEach([&counter](auto a) {
        if (a.value) {
            sink += 2;
        }
        counter++;
        return true;
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated forEach A][" << counter
            << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    auto viewAB = entityManager->createComponentView<ComponentA, ComponentB>();
    start = std::chrono::high_resolution_clock::now();
    for (const auto [a, b]: viewAB) {
        if (a.value) {
            b.value += 1;
            sink += b.value;
        }
        counter++;
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated AB][" << counter <<
            "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    viewAB.forEach([&counter](auto a, auto b) {
        if (a.value) {
            b.value += 1;
            sink += b.value;
        }
        counter++;
        return true;
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated forEach AB][" <<
            counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    auto viewBC = entityManager->createComponentView<ComponentB, const ComponentC>();
    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    for (const auto [b, c]: viewBC) {
        if (c.value) {
            b.value += 1;
            sink += b.value;
        }
        counter++;
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated BC][" << counter <<
            "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    viewBC.forEach([&counter](auto b, auto c) {
        if (c.value > 5) {
            b.value += 1;
            sink += b.value;
        }
        counter++;
        return true;
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated ForEach BC][" <<
            counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    auto viewABC = entityManager->createComponentView<const ComponentA, ComponentB, const ComponentC>();
    start = std::chrono::high_resolution_clock::now();
    for (const auto [a, b, c]: viewABC) {
        if (c.value) {
            b.value += 1;
            sink += b.value;
        }
        counter++;
    }
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated ABC][" << counter <<
            "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    viewABC.forEach([&counter](auto a, auto b, auto c) {
        if (c.value > 3) {
            b.value += 1;
            sink += b.value;
        }
        counter++;
        return true;
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated forEach ABC][" <<
            counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;

    counter = 0;
    auto viewABED = entityManager->createComponentView<const ComponentA, ComponentB, ComponentE, const ComponentD>();
    start = std::chrono::high_resolution_clock::now();
    viewABED.forEach([&counter](auto a, auto b, auto e, auto d) {
        if (a.value) {
            b.value += 1;
            e.value1 += 10;
            sink += b.value;
        }
        counter++;
        return true;
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated forEach ABED][" <<
            counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;


    char a;
    std::cin >> a;
    auto systemAEBF = SystemABEF(entityManager);
    for (auto i = 0; i < 100; i++) {
        systemAEBF.update(1.0f);
    }
    std::cout << systemAEBF.counter << std::endl;


    auto world = World(entityManager);

    const auto system = std::make_shared<SystemABEF>(entityManager);
    world.addUpdatable(system);
    world.unpause();
    world.update(1.0f);

    std::cout << "[" << std::fixed << std::setprecision(3) << world.getLastUpdateTime() << "ms][world with systemAEBF][" << counter << "]"  << std::endl;
    std::cout << "[" << std::fixed << std::setprecision(3) <<  world.getUpdateTimesMs()[0] << "ms][systemAEBF inside world][" << counter << "]"  << std::endl;

    std::cout << "Sink: " << sink << std::endl;
    std::cout << "allocations: " << g_allocationCount << std::endl;
    return 0;
}
