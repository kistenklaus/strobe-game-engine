#include <strobe/core/containers/bitvec.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

TEST(BitVector, resize_preserves_bits_and_clears_new_bits) {
  strobe::BitVector bits(9);
  bits.set(0);
  bits.set(8);

  bits.resize(130);
  EXPECT_EQ(bits.size(), 130);
  EXPECT_TRUE(bits.test(0));
  EXPECT_TRUE(bits.test(8));
  EXPECT_EQ(bits.count(), 2);
  EXPECT_FALSE(bits.test(64));
  EXPECT_FALSE(bits.test(129));

  bits.set(129);
  bits.resize(8);
  EXPECT_EQ(bits.count(), 1);
  EXPECT_TRUE(bits.test(0));

  bits.resize(130);
  EXPECT_EQ(bits.count(), 1); // Removed bits do not return.
  EXPECT_FALSE(bits.test(8));
  EXPECT_FALSE(bits.test(129));
}

TEST(BitVector, operations_work_across_word_boundaries) {
  strobe::BitVector a(130);
  strobe::BitVector b(130);
  a.set(0);
  a.set(64);
  b.set(64);
  b.set(129);

  EXPECT_EQ((a | b).count(), 3);
  EXPECT_EQ((a & b).count(), 1);
  EXPECT_TRUE((a & b).test(64));
  EXPECT_EQ((a ^ b).count(), 2);
  EXPECT_EQ((a - b).count(), 1);
  EXPECT_TRUE((a - b).test(0));
  EXPECT_EQ((~a).count(), 128);
}

TEST(BitVector, whole_set_operations_respect_logical_size) {
  strobe::BitVector<uint8_t> bits(9);

  bits.set_all();
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 9);

  bits.flip_all();
  EXPECT_TRUE(bits.none());

  bits.set_word(1, 0xff);
  EXPECT_EQ(bits.count(), 1);
  EXPECT_TRUE(bits.test(8));

  bits.reset_all();
  EXPECT_TRUE(bits.none());
}

TEST(BitVector, copy_is_independent_and_move_transfers_bits) {
  strobe::BitVector original(70);
  original.set(2);
  original.set(69);

  strobe::BitVector copy(original);
  EXPECT_EQ(copy, original);
  copy.reset(2);
  EXPECT_TRUE(original.test(2));

  strobe::BitVector assigned(1);
  assigned = original;
  EXPECT_EQ(assigned, original);

  strobe::BitVector moved(std::move(copy));
  EXPECT_TRUE(copy.empty());
  EXPECT_EQ(moved.size(), 70);
  EXPECT_FALSE(moved.test(2));
  EXPECT_TRUE(moved.test(69));

  assigned = std::move(moved);
  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.size(), 70);
  EXPECT_TRUE(assigned.test(69));
}

TEST(BitVector, iteration_reports_set_indices) {
  strobe::BitVector bits(130);
  bits.set(129);
  bits.set(0);
  bits.set(64);

  std::vector<size_t> indices;
  bits.for_each_set_bit([&](size_t index) { indices.push_back(index); });

  EXPECT_EQ(indices, (std::vector<size_t>{0, 64, 129}));
}

TEST(BitVector, empty_bitset_can_grow) {
  strobe::BitVector bits;

  EXPECT_TRUE(bits.empty());
  EXPECT_TRUE(bits.none());
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 0);

  bits.resize(1);
  EXPECT_FALSE(bits.test(0));
  bits.set(0);
  EXPECT_TRUE(bits.all());

  bits.resize(0);
  EXPECT_TRUE(bits.empty());
}
