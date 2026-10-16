#include <gtest/gtest.h>
#include <lvtrans/elements/reservoir.hpp>
#include <memory>
#include "lvtrans/elements/constant_level_left.hpp"

using namespace lvtrans;

TEST(ReservoirTest, InitializesRightConnection) {
  auto reservoir = std::make_shared<ConstantLevelLeft>(150.0);
  EXPECT_TRUE(reservoir->left_links().empty());
  ASSERT_EQ(reservoir->right_links().size(), 1u);
  EXPECT_EQ(reservoir->right_links()[0].pipe, nullptr);
}

// TEST(ReservoirTest, KeepsHeadConstantAcrossIterations) {
//   ConstantLevelLeft reservoir(150.0);
//   reservoir.set_c_characteristics(100.0);
//   reservoir.set_b_characteristics(50.0);
//
//   EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
//   for (double time : {0.0, 1.0, 10.0}) {
//     reservoir.iterate(time);
//     EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
//   }
// }
//
// TEST(ReservoirTest, SupportsOutflowEquilibriumAndReverseFlow) {
//   ConstantLevelLeft reservoir(150.0);
//   reservoir.set_b_characteristics(50.0);
//
//   reservoir.set_c_characteristics(100.0);
//   EXPECT_DOUBLE_EQ(reservoir.get_Q(), 1.0);
//
//   reservoir.set_c_characteristics(150.0);
//   EXPECT_DOUBLE_EQ(reservoir.get_Q(), 0.0);
//
//   reservoir.set_c_characteristics(200.0);
//   EXPECT_DOUBLE_EQ(reservoir.get_Q(), -1.0);
//   EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
// }
//
// TEST(ReservoirTest, ModifiesHead) {
//   ConstantLevelLeft reservoir(150.0);
//   EXPECT_DOUBLE_EQ(reservoir.get_H(), 150.0);
//   reservoir.modify(SetReservoirH0{200.0});
//   EXPECT_DOUBLE_EQ(reservoir.get_H(), 200.0);
// }
