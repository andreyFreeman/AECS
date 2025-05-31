//
// Created by ANDREY KLADOV on 24/05/2025.
//

#include <gtest/gtest.h>

#include "ECS/Entity.h"
#include "ECS/Archetype/Chunks/Chunk.hpp"

using namespace ECS::Chunks;

struct TestComponent {
    int x;
    float y;
};

class ChunkTest : public ::testing::Test {
protected:
    static constexpr size_t capacity = 4;
    std::shared_ptr<Chunk> chunk;
    std::vector<TestComponent> compData;

    void SetUp() override {
        compData.resize(capacity);
        std::array<ComponentData, MAX_COMPONENTS> components{};
        ComponentData componentData{reinterpret_cast<char *>(compData.data()), sizeof(TestComponent)};
        components[0] = componentData;
        ECS::Signature signature;
        signature.set(0);
        chunk = std::make_shared<Chunk>(components, 0, capacity, signature);
    }
};

TEST_F(ChunkTest, AddAndSetWorks) {
    TestComponent testValue{42, 3.14f};
    std::array<const void*, MAX_COMPONENTS> data{};
    data[0] = &testValue;

    EXPECT_TRUE(add(*chunk, data));
    chunk->size++;

    EXPECT_EQ(compData[0].x, 42);
    EXPECT_FLOAT_EQ(compData[0].y, 3.14f);

    TestComponent newValue{7, 1.23f};
    data[0] = reinterpret_cast<const char *>(&newValue);
    EXPECT_TRUE(set(*chunk, data, 0));

    EXPECT_EQ(compData[0].x, 7);
    EXPECT_FLOAT_EQ(compData[0].y, 1.23f);

    EXPECT_FALSE(set(*chunk, data, capacity + 1));
}

TEST_F(ChunkTest, SwapWorks) {
    compData[0] = {1, 1.1f};
    compData[1] = {2, 2.2f};
    chunk->size = 2;

    swap(*chunk, 0, 1);

    EXPECT_EQ(compData[0].x, 2);
    EXPECT_FLOAT_EQ(compData[0].y, 2.2f);

    EXPECT_EQ(compData[1].x, 1);
    EXPECT_FLOAT_EQ(compData[1].y, 1.1f);

    swap(*chunk, 0, 0);
    EXPECT_EQ(compData[0].x, 2);
    EXPECT_FLOAT_EQ(compData[0].y, 2.2f);
}
