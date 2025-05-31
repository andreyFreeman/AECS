//
// Created by ANDREY KLADOV on 24/05/2025.
//

#include <gtest/gtest.h>
#include <ECS/Archetype/Chunks/ChunkFactory.hpp>
#include <ECS/Component/ComponentTypeInfo.hpp>

using namespace ECS;

struct A {
    int x;
};

struct B {
    float y;
};

TEST(ChunkFactoryTest, BasicCreation) {
    std::vector<ComponentTypeInfo> types = {
        {0, sizeof(A), alignof(A)},
        {1, sizeof(B), alignof(B)}
    };
    auto registry = std::make_shared<ComponentRegistry>();
    for (const auto& type: types) {
        registry->registerComponent(type);
    }

    const size_t chunkSize = 1024;
    const size_t chunkCount = 2;
    Signature bitset;
    bitset.set(0);
    bitset.set(1);
    ChunkFactory factory(bitset, registry, chunkSize, chunkCount);

    EXPECT_TRUE(factory.canCreateChunk());

    auto chunkOpt = factory.create();
    ASSERT_TRUE(chunkOpt.has_value());

    auto& chunk = chunkOpt.value();

    ASSERT_EQ(chunk.components.size(), MAX_COMPONENTS);
    ASSERT_EQ(chunk.size, 0);
    ASSERT_GT(chunk.capacity, 0);

    for (auto i = bitset.lowestBit; i <= bitset.highestBit; i++) {
        if (bitset[i]) {
            EXPECT_NE(chunk.components[i].ptr, nullptr);
            EXPECT_NE(chunk.components[i].stride, 0);
        } else {
            EXPECT_EQ(chunk.components[i].ptr, nullptr);
            EXPECT_EQ(chunk.components[i].stride, 0);
        }
    }

    for (size_t i = 1; i < chunkCount; ++i) {
        EXPECT_TRUE(factory.canCreateChunk());
        EXPECT_TRUE(factory.create().has_value());
    }

    EXPECT_FALSE(factory.canCreateChunk());
    EXPECT_FALSE(factory.create().has_value());
}