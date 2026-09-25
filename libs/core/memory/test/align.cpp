#include <gtest/gtest.h>

#include <cstddef>

#include <strobe/core/memory/align.hpp>

namespace {

using strobe::memory::align_down;
using strobe::memory::align_up;

static_assert(align_up(0, 8) == 0);
static_assert(align_up(1, 8) == 8);
static_assert(align_up(16, 8) == 16);
static_assert(align_down(0, 8) == 0);
static_assert(align_down(7, 8) == 0);
static_assert(align_down(16, 8) == 16);

TEST(Align, AlignUpRoundsToNextBoundary) {
  EXPECT_EQ(align_up(1, 8), 8U);
  EXPECT_EQ(align_up(8, 8), 8U);
  EXPECT_EQ(align_up(9, 8), 16U);
}

TEST(Align, AlignDownRoundsToPreviousBoundary) {
  EXPECT_EQ(align_down(1, 8), 0U);
  EXPECT_EQ(align_down(8, 8), 8U);
  EXPECT_EQ(align_down(15, 8), 8U);
}

TEST(Align, SupportsDifferentPowerOfTwoAlignments) {
  EXPECT_EQ(align_up(33, 32), 64U);
  EXPECT_EQ(align_down(63, 32), 32U);
  EXPECT_EQ(align_up(257, 256), 512U);
  EXPECT_EQ(align_down(511, 256), 256U);
}

} // namespace
