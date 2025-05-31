//
//  main.cpp
//  ecs_runner
//
//  Created by ANDREY KLADOV on 13/05/2025.
//

#include <ECS/EntityManager.hpp>
#include <iostream>
#include <iomanip>
#include <ECS/System/SystemComponentView.hpp>

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

class SystemABEF final : public ECS::SystemComponentView<const ComponentA, ComponentB, ComponentE, const ComponentD> {
    using SystemComponentView::SystemComponentView;

public:
    int counter = 0;

    bool update(float dt) override {
        auto updated = false;
        auto start = std::chrono::high_resolution_clock::now();
        componentView.forEach([&](auto a, auto b, auto e, auto d) {
            if (a.value) {
                b.value += 1;
                e.value1 += 10;
            }
            updated = b.value % 3;
            counter++;
        });
        auto enumerated = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
        std::cout << "[" << enumerated.count() << "][ns][" << counter << "][" << static_cast<double>(enumerated.count()) / counter << "]" <<
                std::endl;
        counter = 0;
        return updated;
    }
};

static constexpr auto entitiesToAdd = 3300000;

static size_t g_allocationCount = 0;

// void* operator new(std::size_t size) {
//     ++g_allocationCount;
//     std::cout << "[new] Allocating " << size << " bytes. Total allocations: " << g_allocationCount << "\n";
//     void* p = std::malloc(size);
//     if (!p) {
//         throw std::bad_alloc();
//     }
//     return p;
// }
//
// void operator delete(void* p) noexcept {
//     // --g_allocationCount;
//     // std::cout << "[delete] Freeing memory. Remaining allocations: " << g_allocationCount << "\n";
//     std::free(p);
// }

int main(int argc, const char *argv[]) {
    auto counter = 0;
    static float sink = 0;
    const auto entityManager = std::make_unique<ECS::EntityManager>();

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
            entityManager->setComponents<ComponentD, ComponentE>(i, {10.0 * i}, {static_cast<float>(10.0) * i, 10 * i, false});
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
    });
    elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
    elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "[" << std::fixed << std::setprecision(3) << elapsedMillis.count() << "ms][" << elapsedNanos.count() << "ns][Iterated forEach ABED][" <<
            counter << "]";
    std::cout << "[" << static_cast<double>(elapsedNanos.count()) / counter << "]" << std::endl;


    char a;
    std::cin >> a;
    auto systemAEBF = SystemABEF(*entityManager.get());
    for (auto i = 0; i < 100; i++) {
        systemAEBF.update(1.0f);
    }
    std::cout << systemAEBF.counter << std::endl;
    std::cout << "Sink: " << sink << std::endl;
    std::cout << "allocations: " << g_allocationCount << std::endl;
    return 0;
}
