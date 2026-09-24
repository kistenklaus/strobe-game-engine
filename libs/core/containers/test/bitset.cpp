#include <strobe/core/containers/bitset.hpp>

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

TEST(Bitset, individual_bits_across_word_boundary) {
  strobe::Bitset<130> bits;

  EXPECT_TRUE(bits.none());
  bits.set(0);
  bits.set(63);
  bits.set(64);
  bits.set(129);

  EXPECT_EQ(bits.count(), 4);
  EXPECT_TRUE(bits.test(0));
  EXPECT_TRUE(bits[63]);
  EXPECT_TRUE(bits.test(64));
  EXPECT_TRUE(bits.test(129));
  EXPECT_FALSE(bits.test(65));

  bits.reset(63);
  bits.flip(64);
  bits.flip(65);

  EXPECT_FALSE(bits.test(63));
  EXPECT_FALSE(bits.test(64));
  EXPECT_TRUE(bits.test(65));
  EXPECT_EQ(bits.count(), 3);
}

TEST(Bitset, whole_set_operations_respect_bit_count) {
  strobe::Bitset<65> bits;

  bits.set_all();
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 65);

  bits.flip_all();
  EXPECT_TRUE(bits.none());

  bits.flip_all();
  EXPECT_TRUE(bits.all());

  bits.reset_all();
  EXPECT_TRUE(bits.none());
}

TEST(Bitset, bitwise_operations) {
  strobe::Bitset<65> a;
  strobe::Bitset<65> b;
  a.set(0);
  a.set(64);
  b.set(1);
  b.set(64);

  EXPECT_EQ((a | b).count(), 3);
  EXPECT_EQ((a & b).count(), 1);
  EXPECT_TRUE((a & b).test(64));
  EXPECT_EQ((a ^ b).count(), 2);
  EXPECT_EQ((a - b).count(), 1);
  EXPECT_TRUE((a - b).test(0));

  EXPECT_EQ((~a).count(), 63);
  EXPECT_EQ(a | b, b | a);

  a.clear_bits(b);
  EXPECT_EQ(a.count(), 1);
  EXPECT_TRUE(a.test(0));
}

TEST(Bitset, iterates_each_set_bit_once) {
  strobe::Bitset<130> bits;
  bits.set(129);
  bits.set(0);
  bits.set(64);

  std::vector<size_t> visited;
  bits.for_each_set_bit([&](size_t index) { visited.push_back(index); });

  EXPECT_EQ(visited, (std::vector<size_t>{0, 64, 129}));
}

TEST(Bitset, set_word_masks_bits_past_end) {
  strobe::Bitset<9, uint8_t> bits;
  bits.set_word(1, 0xff);

  EXPECT_EQ(bits.count(), 1);
  EXPECT_TRUE(bits.test(8));
  EXPECT_TRUE(bits.all() == false);

  bits.set_word(0, 0xff);
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 9);
}

TEST(Bitset, exactly_one_word) {
  strobe::Bitset<64> bits = strobe::Bitset<64>::full();

  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 64);
  bits.reset(63);
  EXPECT_FALSE(bits.all());
  EXPECT_EQ(bits.count(), 63);
}

static_assert([] {
  strobe::Bitset<9> bits;
  bits.set(8);
  return bits.test(8) && bits.count() == 1;
}());
