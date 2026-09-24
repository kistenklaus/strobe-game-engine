#include <strobe/core/containers/boxed_bitset.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>
#include <vector>

TEST(BoxedBitset, constructs_with_fixed_runtime_size) {
  strobe::BoxedBitset bits(130);

  EXPECT_EQ(bits.size(), 130);
  EXPECT_TRUE(bits.none());
  EXPECT_EQ(bits.count(), 0);

  bits.set(0);
  bits.set(64);
  bits.set(129);

  EXPECT_EQ(bits.count(), 3);
  EXPECT_TRUE(bits.test(0));
  EXPECT_TRUE(bits.test(64));
  EXPECT_TRUE(bits.test(129));
  EXPECT_FALSE(bits.test(65));
}

TEST(BoxedBitset, whole_set_operations_mask_final_word) {
  strobe::BoxedBitset<uint8_t> bits(9);

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

TEST(BoxedBitset, bitwise_operations) {
  strobe::BoxedBitset a(70);
  strobe::BoxedBitset b(70);
  a.set(0);
  a.set(64);
  b.set(64);
  b.set(69);

  EXPECT_EQ((a | b).count(), 3);
  EXPECT_EQ((a & b).count(), 1);
  EXPECT_TRUE((a & b).test(64));
  EXPECT_EQ((a ^ b).count(), 2);
  EXPECT_EQ((a - b).count(), 1);
  EXPECT_TRUE((a - b).test(0));
  EXPECT_EQ((~a).count(), 68);
}

TEST(BoxedBitset, copy_has_independent_storage) {
  strobe::BoxedBitset original(70);
  original.set(2);
  original.set(69);

  strobe::BoxedBitset copy(original);
  EXPECT_EQ(copy, original);
  EXPECT_NE(copy.data(), original.data());

  copy.reset(2);
  EXPECT_TRUE(original.test(2));

  strobe::BoxedBitset assigned(1);
  assigned = original;
  EXPECT_EQ(assigned, original);
  EXPECT_NE(assigned.data(), original.data());
}

TEST(BoxedBitset, move_transfers_storage) {
  strobe::BoxedBitset source(70);
  source.set(69);
  const auto *storage = source.data();

  strobe::BoxedBitset moved(std::move(source));
  EXPECT_TRUE(source.empty());
  EXPECT_EQ(moved.data(), storage);
  EXPECT_TRUE(moved.test(69));

  strobe::BoxedBitset assigned(1);
  assigned = std::move(moved);
  EXPECT_TRUE(moved.empty());
  EXPECT_EQ(assigned.data(), storage);
  EXPECT_TRUE(assigned.test(69));
}

TEST(BoxedBitset, iterates_set_indices) {
  strobe::BoxedBitset bits(130);
  bits.set(129);
  bits.set(0);
  bits.set(64);

  std::vector<size_t> indices;
  bits.for_each_set_bit([&](size_t index) {
    indices.push_back(index);
  });

  EXPECT_EQ(indices, (std::vector<size_t>{0, 64, 129}));
}

TEST(BoxedBitset, zero_size_is_empty) {
  strobe::BoxedBitset bits(0);

  EXPECT_TRUE(bits.empty());
  EXPECT_TRUE(bits.none());
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 0);
  EXPECT_EQ(bits.word_count(), 0);
}
