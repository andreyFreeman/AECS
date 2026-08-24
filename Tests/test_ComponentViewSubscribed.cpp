//
// Created by ANDREY KLADOV on 24/08/2026.
//

#include <gtest/gtest.h>
#include <ECS/EntityManager.hpp>

using namespace ECS;

struct Position {
    float x, y;
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
};

struct Velocity {
    float dx, dy;
    bool operator==(const Velocity& other) const { return dx == other.dx && dy == other.dy; }
};

struct Health {
    int value;
    bool operator==(const Health& other) const { return value == other.value; }
};

#define COUNT_ENTITIES(view, count_var) \
    int count_var = 0; \
    view.forEach([&count_var](const auto&, const auto&) { \
        count_var++; \
        return false; \
    })

TEST(ComponentViewSubscribedTest, ComponentViewSubscribedIncludesNewArchtypes) {
    EntityManager manager{};
    auto componentView = manager.createComponentView<Position, Velocity>();

    manager.createWithComponents(
        Position{1.0f, 2.0f},
        Velocity{0.0f, 0.0f}
    );

    COUNT_ENTITIES(componentView, count1);
    EXPECT_EQ(count1, 1) << "View should see the first entity.";

    manager.createWithComponents(
        Position{3.0f, 4.0f},
        Velocity{1.0f, 1.0f},
        Health{100}
    );

    COUNT_ENTITIES(componentView, count2);
    EXPECT_EQ(count2, 2) << "View should dynamically include the newly created archetype.";
}

TEST(ComponentViewSubscribedTest, ComponentViewSubscribedExcludesCorrectly) {
    EntityManager manager{};

    auto componentViewWithExcluded = manager.createComponentViewWithQuery<Position, Velocity>(
        EntityManager::Query<Position, Velocity>{},
        EntityManager::Query<Health>{}
    );

    manager.createWithComponents(
        Position{1.0f, 2.0f},
        Velocity{0.0f, 0.0f}
    );

    manager.createWithComponents(
        Position{3.0f, 4.0f},
        Velocity{1.0f, 1.0f},
        Health{100}
    );

    int count = 0;
    float foundX = 0.0f;
    componentViewWithExcluded.forEach([&](const auto& pos, const auto& vel) {
        count++;
        foundX = pos.x;
        return false;
    });

    EXPECT_EQ(count, 1) << "View should only iterate over 1 entity due to exclusion rule.";
    EXPECT_EQ(foundX, 1.0f) << "View iterated over the wrong entity.";
}

TEST(ComponentViewSubscribedTest, ComponentViewSubscribedExcludesDataMigratedOut) {
    EntityManager manager{};

    auto componentViewWithExcluded = manager.createComponentViewWithQuery<Position, Velocity>(
        EntityManager::Query<Position, Velocity>{},
        EntityManager::Query<Health>{}
    );

    const auto entity = manager.createWithComponents(
        Position{1.0f, 2.0f},
        Velocity{0.0f, 0.0f}
    );

    COUNT_ENTITIES(componentViewWithExcluded, countBefore);
    EXPECT_EQ(countBefore, 1) << "Entity should be visible before migration.";

    manager.setComponent(entity, Health{50});

    COUNT_ENTITIES(componentViewWithExcluded, countAfter);
    EXPECT_EQ(countAfter, 0) << "Entity should disappear from the view after acquiring an excluded component.";
}

TEST(ComponentViewSubscribedTest, ComponentViewSubscribedIncludesDataMigratedIn) {
    EntityManager manager{};

    auto componentViewWithExcluded = manager.createComponentViewWithQuery<Position, Velocity>(
        EntityManager::Query<Position, Velocity>{},
        EntityManager::Query<Health>{}
    );

    const auto entity = manager.createWithComponents(
        Position{1.0f, 2.0f},
        Velocity{0.0f, 0.0f},
        Health{50}
    );

    COUNT_ENTITIES(componentViewWithExcluded, countBefore);
    EXPECT_EQ(countBefore, 0) << "Entity should NOT be visible initially.";

    manager.removeComponent<Health>(entity);

    COUNT_ENTITIES(componentViewWithExcluded, countAfter);
    EXPECT_EQ(countAfter, 1) << "Entity should dynamically appear in the view after the excluded component is removed.";
}

TEST(ComponentViewSubscribedTest, ComponentViewIteratorFunctionsCorrectly) {
    EntityManager manager{};
    auto componentView = manager.createComponentView<Position, Velocity>();

    manager.createWithComponents(Position{1.0f, 1.0f}, Velocity{1.0f, 1.0f});
    manager.createWithComponents(Position{2.0f, 2.0f}, Velocity{2.0f, 2.0f});

    int count = 0;
    for (auto it = componentView.begin(); it != componentView.end(); ++it) {
        count++;
    }

    EXPECT_EQ(count, 2) << "Iterator should successfully traverse all elements.";
}