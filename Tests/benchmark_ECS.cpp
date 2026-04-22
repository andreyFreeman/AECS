//
// Created by ANDREY KLADOV on 24/05/2025.
//

#include <memory>
#include <random>
#include <benchmark/benchmark.h>
#include "ECS/EntityManager.hpp"
#include "random.h"
#include "Systems.hpp"

static void BM_createEntities(benchmark::State &state) {
    const auto entities = state.range(0);
    auto entityManager = ECS::EntityManager();
    for (auto _: state) {
        for (auto i = 0; i < entities; i++) {
            entityManager.createWithComponents(PositionComponent(), VelocityComponent(), SpriteComponent());
        }
    }
    state.SetItemsProcessed(state.iterations() * entities);
}

static void BM_iterateEntitiesWith1ComponentWithForEach(benchmark::State &state) {
    const auto entities = state.range(0);
    auto entityManager = ECS::EntityManager();
    for (auto i = 0; i < entities; i++) {
        const auto entity = entityManager.createWithComponents(PositionComponent(), SpriteComponent());
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        if (dis(gen) > 0.1) {
            entityManager.setComponent(entity, DataComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager.setComponent(entity, DamageComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager.setComponent(entity, HealthComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager.setComponent(entity, VelocityComponent{});
        }
    }
    auto view = entityManager.createComponentView<PositionComponent>();
    for (auto _: state) {
        view.forEach([&](auto &position) {
            position.x += 0.03;
            position.y += 0.03;
            return true;
        });
    }
    state.SetItemsProcessed(state.iterations() * entities);
}

static void BM_updateEntitiesWith6ComponentsWithForEach(benchmark::State &state) {
    const auto entities = state.range(0);
    auto entityManager = std::make_shared<ECS::EntityManager>();
    for (auto i = 0; i < entities; i++) {
        const auto entity = entityManager->createWithComponents(PositionComponent(), SpriteComponent());
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, DataComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, DamageComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, HealthComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, VelocityComponent{});
        }
    }

    auto view = entityManager->createComponentView<PositionComponent, SpriteComponent, DataComponent, DamageComponent, HealthComponent,
        VelocityComponent>();
    for (auto _: state) {
        view.forEach([&](auto &position, auto &sprite, auto &data, auto &damage, auto &health, auto &velocity) {
            if (health.hp > 0) {
                position.x += velocity.x;
                position.y += velocity.y;
            }
            if (damage.atk > 0) {
                {
                    health.hp -= damage.atk;
                }
            }
            return true;
        });
    }
    state.SetItemsProcessed(state.iterations() * entities);
}

static void BM_updateEntitiesWithMultipleSystems(benchmark::State &state) {
    const auto entities = state.range(0);
    auto entityManager = std::make_shared<ECS::EntityManager>();
    for (auto i = 0; i < entities; i++) {
        const auto entity = entityManager->createWithComponents(PositionComponent(), SpriteComponent());
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, DataComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, DamageComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, HealthComponent{});
        }
        if (dis(gen) > 0.1) {
            entityManager->setComponent(entity, VelocityComponent{});
        }
    }

    const std::array<std::unique_ptr<Updatable>, 7> systems{
        {
            std::make_unique<MovementSystem>(entityManager),
            std::make_unique<DataSystem>(entityManager),
            std::make_unique<MoreComplexSystem>(entityManager),
            std::make_unique<HealthSystem>(entityManager),
            std::make_unique<DamageSystem>(entityManager),
            std::make_unique<SpriteSystem>(entityManager),
            std::make_unique<RenderSystem>(entityManager)
        }
    };

    for (auto _: state) {
        for (auto &system: systems) {
            system->update(0.08);
        }
    }
    state.SetItemsProcessed(state.iterations() * entities);
}

BENCHMARK(BM_createEntities)->RangeMultiplier(4)->Range(1, 4194304)->Iterations(1);
BENCHMARK(BM_iterateEntitiesWith1ComponentWithForEach)->RangeMultiplier(4)->Range(1, 2000000)->Iterations(10);
BENCHMARK(BM_updateEntitiesWith6ComponentsWithForEach)->RangeMultiplier(4)->Range(1, 2000000)->Iterations(10);
BENCHMARK(BM_updateEntitiesWithMultipleSystems)->RangeMultiplier(4)->Range(1, 2000000)->Iterations(10);
