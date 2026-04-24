# AECS - Archetyped Entity Component System

AECS is a high-performance, cache-friendly C++ Entity Component System designed for real-time applications like games and simulations. It is minimal, efficient, and focused on data-oriented design.

---

## ✨ Features

- **Header-only**
- Extremely **lightweight**
- Modern **C++23**
- Lightning-fast: **~0.4 to 2 ns per entity**
- Fast archetype-based storage
- Iterators and `forEach` API
- Flexible component views

---

## Examples

### Iteration
```c++
auto view = entityManager->createComponentView<const ECS::Entity, const A, C>();

// functional forEach iteration
view.forEach([](const ECS::Entity entity, const A &a, C &c) {
    if (!a.value) {
        c.y -= dt;
        return true;
    }
    return false;
});

// range iteratoration with tuples 
for (const auto&[a, c]: view) {
    if (a.value) {
        c.x += dt;
    }
}
```

### EntityManager
```c++
#include <ECS/EntityManager.hpp>

struct A {
    bool value;
};

struct B {
    int value;
};

struct C {
    float x;
    float y;
};

auto entityManager = ECS::EntityManager();

// Create entities with components sets:
auto entity1 = entityManager.createWithComponents(A{true}, B{1}, C{0.5f, 1.0f});

// add/replace components
const auto entity2 = entityManager.create();
entityManager.setComponent(entity2, C{1.0f, 2.5f});
entityManager.setComponents(entity1, B{}, C{});

// remove components
entityManager.removeComponent<B>(entity1);

// access components
auto& a = entityManager.getComponent<C>(entity2);

// create component views
auto view = entityManager.createComponentView<const A, C>();

// range iteration with tuples
for (const auto& [a, c]: view) {
    if (a.value) {
        c.x += dt;
    }
}

// forEach iteration
view.forEach([](const A &a, C &c) {
    if (!a.value) {
        c.y -= dt;
    }
});
```
### System
```c++
class SystemABC final : public ECS::SystemComponentView<const ECS::Entity, const A, B, C> {
    bool update(float dt) override {
        return componentView.forEach([&](const ECS::Entity e, const A &a, B &b, C &c) {
            if (a.value) {
                c.x += b.value * dt;
                return true;
            }
            return false;
        });
    }
};
```

### World
```c++
auto entityManager = std::make_shared<ECS::EntityManager>();
auto system = std::make_shared<SystemABC>(entityManager);
auto world = std::make_shared<ECS::World>(entityManager);
world->addUpdatable(system);
world->update(0.08f);
```

## Performance
- Update time: **~2 ns per entity** in realistic setups 
- Component iteration: Optimized for both **speed and cache locality** 
- Creation/removal/migration: Efficient at all scales

## Storage
- Data is stored in 128 KB chunks (configurable)
- Each chunk holds arrays-of-structs for each component type 
- Entities with the same component mask are stored together for fast iteration 
- Maximum component count is configurable (128+ is possible with negligible overhead)
