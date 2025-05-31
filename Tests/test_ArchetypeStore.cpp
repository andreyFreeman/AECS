//
// Created by ANDREY KLADOV on 24/05/2025.
//

#include <gtest/gtest.h>
#include <ECS/Archetype/ArchetypeStore.hpp>
#include <ECS/Entity.h>

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

class ArchetypeStoreTest : public ::testing::Test {
protected:
    ArchetypeStore store;
    Entity entity1 = Entity{1};
    Entity entity2 = Entity{2};
};

TEST_F(ArchetypeStoreTest, InitialComponentSetupWorks) {
    bool added = store.setComponents(entity1, Position{1.0f, 2.0f});
    EXPECT_TRUE(added);

    auto* pos = store.getComponent<Position>(entity1);
    ASSERT_NE(pos, nullptr);
    Position expected{1.0f, 2.0f};
    EXPECT_EQ(*pos, expected);
}

TEST_F(ArchetypeStoreTest, PreventsDuplicateInitialSetup) {
    EXPECT_TRUE(store.setComponents(entity1, Position{0, 0}));
    EXPECT_TRUE(store.setComponents(entity1, Velocity{1, 1}));
}

TEST_F(ArchetypeStoreTest, AddComponentCreatesNewArchetype) {
    store.setComponents(entity1, Position{3.0f, 4.0f});
    EXPECT_TRUE(store.setComponents(entity1, Velocity{1.0f, 2.0f}));

    EXPECT_TRUE(store.hasComponent<Velocity>(entity1));
    auto* vel = store.getComponent<Velocity>(entity1);
    ASSERT_NE(vel, nullptr);
    auto expected = Velocity{1.0f, 2.0f};
    EXPECT_EQ(*vel, expected);
}

TEST_F(ArchetypeStoreTest, RemoveComponentMovesEntity) {
    store.setComponents(entity1, Position{1.0f, 2.0f}, Velocity{3.0f, 4.0f});
    EXPECT_TRUE(store.removeComponent<Velocity>(entity1));

    EXPECT_FALSE(store.hasComponent<Velocity>(entity1));
    EXPECT_TRUE(store.hasComponent<Position>(entity1));
}

TEST_F(ArchetypeStoreTest, RemoveLastComponentRemovesEntity) {
    store.setComponents(entity1, Position{1.0f, 2.0f});
    EXPECT_TRUE(store.removeComponent<Position>(entity1));
    EXPECT_FALSE(store.hasComponent<Position>(entity1));
    EXPECT_EQ(store.getComponent<Position>(entity1), nullptr);
}

TEST_F(ArchetypeStoreTest, RemoveEntityWorks) {
    store.setComponents(entity1, Health{100});
    EXPECT_TRUE(store.removeEntity(entity1));
    EXPECT_FALSE(store.hasComponent<Health>(entity1));
}

TEST_F(ArchetypeStoreTest, GetComponentReturnsNullIfMissing) {
    EXPECT_EQ(store.getComponent<Position>(entity1), nullptr);
    store.setComponents(entity1, Health{100});
    EXPECT_EQ(store.getComponent<Velocity>(entity1), nullptr);
}

TEST_F(ArchetypeStoreTest, NotifierTriggersOnAddAndUpdate) {
    bool addCalled = false;
    bool updateCalled = false;

    store.getChangeNotifier()->subscribeToAdd([&](const auto&) {
        addCalled = true;
    });

    store.getChangeNotifier()->subscribeToUpdate([&](const auto&) {
        updateCalled = true;
    });

    store.setComponents(entity1, Position{0, 0});
    store.setComponents(entity1, Velocity{0.5f, 0.5f});

    EXPECT_TRUE(addCalled);
    EXPECT_TRUE(updateCalled);
}

TEST_F(ArchetypeStoreTest, FindArchetypesReturnsMatches) {
    store.setComponents(entity1, Position{1, 2});
    store.setComponents(entity2, Position{3, 4}, Velocity{1, 1});

    auto signature = Signature();
    signature.set(1);
    auto results = store.findArchetypes(signature);

    EXPECT_EQ(results.size(), 2);
}