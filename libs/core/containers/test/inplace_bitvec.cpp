#include <strobe/core/containers/inplace_bitvec.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

TEST(InplaceBitVector, starts_empty_and_grows_with_clear_bits) {
  strobe::InplaceBitVector<130> bits;

  EXPECT_TRUE(bits.empty());
  EXPECT_EQ(bits.capacity, 130);
  EXPECT_TRUE(bits.none());

  bits.resize(130);
  EXPECT_EQ(bits.size(), 130);
  EXPECT_EQ(bits.count(), 0);

  bits.set(0);
  bits.set(64);
  bits.set(129);
  EXPECT_EQ(bits.count(), 3);
  EXPECT_TRUE(bits.test(64));
}

TEST(InplaceBitVector, shrinking_discards_bits) {
  strobe::InplaceBitVector<130> bits(130);
  bits.set(0);
  bits.set(8);
  bits.set(64);
  bits.set(129);

  bits.resize(8);
  EXPECT_EQ(bits.count(), 1);
  EXPECT_TRUE(bits.test(0));

  bits.resize(130);
  EXPECT_EQ(bits.count(), 1);
  EXPECT_FALSE(bits.test(8));
  EXPECT_FALSE(bits.test(64));
  EXPECT_FALSE(bits.test(129));
}

TEST(InplaceBitVector, resizing_within_a_word_clears_removed_bits) {
  strobe::InplaceBitVector<16, uint8_t> bits(7);
  bits.set(6);

  bits.resize(3);
  bits.resize(7);

  EXPECT_FALSE(bits.test(6));
  EXPECT_EQ(bits.count(), 0);
}

TEST(InplaceBitVector, whole_set_operations_mask_final_word) {
  strobe::InplaceBitVector<16, uint8_t> bits(9);

  bits.set_all();
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 9);

  bits.flip_all();
  EXPECT_TRUE(bits.none());

  bits.set_word(1, 0xff);
  EXPECT_EQ(bits.count(), 1);
  EXPECT_TRUE(bits.test(8));
}

TEST(InplaceBitVector, bitwise_operations_require_matching_sizes) {
  strobe::InplaceBitVector<70> a(70);
  strobe::InplaceBitVector<70> b(70);
  a.set(0);
  a.set(64);
  b.set(64);
  b.set(69);

  EXPECT_EQ((a | b).count(), 3);
  EXPECT_EQ((a & b).count(), 1);
  EXPECT_TRUE((a & b).test(64));
  EXPECT_EQ((a ^ b).count(), 2);
  EXPECT_EQ((a - b).count(), 1);
  EXPECT_EQ((~a).count(), 68);

  b.resize(69);
  EXPECT_NE(a, b);
}

TEST(InplaceBitVector, iterates_set_indices) {
  strobe::InplaceBitVector<130> bits(130);
  bits.set(129);
  bits.set(0);
  bits.set(64);

  std::vector<size_t> indices;
  bits.for_each_set_bit([&](size_t index) { indices.push_back(index); });

  EXPECT_EQ(indices, (std::vector<size_t>{0, 64, 129}));
}

TEST(InplaceBitVector, zero_capacity_is_empty) {
  strobe::InplaceBitVector<0> bits;

  EXPECT_EQ(bits.size(), 0);
  EXPECT_EQ(bits.word_count(), 0);
  EXPECT_TRUE(bits.none());
  EXPECT_TRUE(bits.all());
}
