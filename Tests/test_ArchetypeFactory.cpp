//
// Created by ANDREY KLADOV on 24/05/2025.
//

#include <gtest/gtest.h>
#include <ECS/Archetype/ArchetypeFactory.hpp>

using namespace ECS;

struct A { int a, b, c; };
struct B { float a, b, c; };
struct C { bool a; };

TEST(ArchetypeFactoryTest, CreateStaticArchetypeIncludesEntity) {
    const std::vector<ComponentTypeInfo> types{
                    {
                        {0, sizeof(A), alignof(A)},
                        {1, sizeof(B), alignof(B)}
                    }};
    auto registry = std::make_shared<ComponentRegistry>();
    auto factory = ArchetypeFactory(registry);
    auto signature = Signature();
    for (const auto& type: types) {
        registry->registerComponent(type);
        signature.set(type.type);
    }
    auto archetype = factory.createArchetypeDynamic(signature);

    EXPECT_TRUE(archetype->getSignature().test(0));
    EXPECT_TRUE(archetype->getSignature().test(1));
    EXPECT_FALSE(archetype->getSignature().test(2));
}

TEST(ArchetypeFactoryTest, CreateStaticArchetypeTypesOrderedCorrectly) {

    std::unordered_map<ComponentType, std::pair<size_t, size_t>> expectedInfo = {
        { 0, { sizeof(Entity), alignof(Entity) } },
        { 1, { sizeof(A), alignof(A) } },
        { 2, { sizeof(B), alignof(B) } },
        { 3, { sizeof(C), alignof(C) } },
    };

    const std::vector<ComponentTypeInfo> types{
                        {
                            {0, sizeof(Entity), alignof(Entity)},
                            {1, sizeof(A), alignof(A)},
                            {2, sizeof(B), alignof(B)},
                            {3, sizeof(C), alignof(C)}
                        }};

    auto registry = std::make_shared<ComponentRegistry>();
    auto factory = ArchetypeFactory(registry);
    auto signature = Signature();
    for (const auto& type: types) {
        registry->registerComponent(type);
        signature.set(type.type);
    }
    auto archetype = factory.createArchetypeDynamic(signature);
    auto archetypeSignature = archetype->getSignature();
    EXPECT_TRUE(archetypeSignature.test(0));
    EXPECT_TRUE(archetypeSignature.test(1));
    EXPECT_TRUE(archetypeSignature.test(2));
    EXPECT_TRUE(archetypeSignature.test(3));
    EXPECT_FALSE(archetypeSignature.test(4));
    EXPECT_FALSE(archetypeSignature.test(5));
}