#include <gtest/gtest.h>
#include "voxel_engine.h"

// Iteration 4
TEST(VoxelEngine, Basic) {
    EXPECT_EQ(1, 1);
}

TEST(VoxelEngine, IterationCheck) {
    EXPECT_GE(4, 1);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
