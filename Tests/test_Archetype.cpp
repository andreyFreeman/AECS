//
// Created by ANDREY KLADOV on 24/05/2025.
//

#include <gtest/gtest.h>
#include <ECS/Archetype/ArchetypeFactory.hpp>

using namespace ECS;

struct A {
    int64_t x;
};

struct B {
    double y;
};

const std::vector<ComponentTypeInfo> types{
    {
        {0, sizeof(Entity), alignof(Entity)},
        {1, sizeof(A), alignof(A)},
        {2, sizeof(B), alignof(B)}
    }
};

TEST(ArchetypeTest, AddAndAccessEntities) {
    auto registry = std::make_shared<ComponentRegistry>();
    auto factory = ArchetypeFactory(registry);
    auto signature = Signature();
    for (const auto& type: types) {
        registry->registerComponent(type);
        signature.set(type.type);
    }

    auto archetype = factory.createArchetypeDynamic(signature);
    EXPECT_TRUE(archetype->empty());

    Entity e1 = 1;
    A a{42};
    B b{3.14f};

    std::array<const void*, 3> data{(&e1), (&a), (&b)};

    EXPECT_TRUE(archetype->set(data));
    EXPECT_EQ(archetype->size(), 1);
    EXPECT_FALSE(archetype->empty());

    auto *ptrE = static_cast<Entity *>(archetype->getComponent(e1, 0));
    auto *ptrA = static_cast<A *>(archetype->getComponent(e1, 1));
    auto *ptrB = static_cast<B *>(archetype->getComponent(e1, 2));

    ASSERT_NE(ptrE, nullptr);
    ASSERT_NE(ptrA, nullptr);
    ASSERT_NE(ptrB, nullptr);

    EXPECT_EQ(*ptrE, e1);
    EXPECT_EQ(ptrA->x, 42);
    EXPECT_FLOAT_EQ(ptrB->y, 3.14f);
}

TEST(ArchetypeTest, RemoveEntity) {
    auto registry = std::make_shared<ComponentRegistry>();
    auto factory = ArchetypeFactory(registry);
    auto signature = Signature();
    for (const auto& type: types) {
        registry->registerComponent(type);
        signature.set(type.type);
    }
    auto archetype = factory.createArchetypeDynamic(signature);

    A a{99};
    B b{1.0f};
    Entity e1 = 10;
    Entity e2 = 11;

    std::array<const void*, 3> data{ &e1, &a, &b};

    EXPECT_TRUE(archetype->set(data));
    data[0] = reinterpret_cast<const char *>(&e2);
    EXPECT_TRUE(archetype->set(data));
    EXPECT_EQ(archetype->size(), 2);

    EXPECT_TRUE(archetype->remove(e1));
    EXPECT_EQ(archetype->size(), 1);
}
