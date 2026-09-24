#include <strobe/core/containers/small_bitvec.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>
#include <vector>

TEST(SmallBitVector, stays_inline_then_grows_to_heap) {
  strobe::SmallBitVector<64> bits;

  EXPECT_TRUE(bits.using_inline_storage());
  EXPECT_GE(bits.inline_capacity(), 64);

  bits.resize(64);
  bits.set(0);
  bits.set(63);
  EXPECT_TRUE(bits.using_inline_storage());

  bits.resize(65);
  EXPECT_FALSE(bits.using_inline_storage());
  EXPECT_TRUE(bits.test(0));
  EXPECT_TRUE(bits.test(63));
  EXPECT_FALSE(bits.test(64));
}

TEST(SmallBitVector, shrinking_discards_bits_and_can_return_inline) {
  strobe::SmallBitVector<64> bits(130);
  ASSERT_FALSE(bits.using_inline_storage());

  bits.set(0);
  bits.set(63);
  bits.set(64);
  bits.set(129);

  bits.resize(64);
  EXPECT_EQ(bits.count(), 2);
  EXPECT_TRUE(bits.test(0));
  EXPECT_TRUE(bits.test(63));

  bits.shrink_to_fit();
  EXPECT_TRUE(bits.using_inline_storage());

  bits.resize(130);
  EXPECT_EQ(bits.count(), 2);
  EXPECT_FALSE(bits.test(64));
  EXPECT_FALSE(bits.test(129));
}

TEST(SmallBitVector, shrinking_within_a_word_clears_removed_bits) {
  strobe::SmallBitVector<16, uint8_t> bits(7);
  bits.set(6);

  bits.resize(3);
  bits.resize(7);

  EXPECT_FALSE(bits.test(6));
  EXPECT_EQ(bits.count(), 0);
}

TEST(SmallBitVector, whole_set_operations_mask_final_word) {
  strobe::SmallBitVector<16, uint8_t> bits(9);

  bits.set_all();
  EXPECT_TRUE(bits.all());
  EXPECT_EQ(bits.count(), 9);

  bits.flip_all();
  EXPECT_TRUE(bits.none());

  bits.set_word(1, 0xff);
  EXPECT_EQ(bits.count(), 1);
  EXPECT_TRUE(bits.test(8));
}

TEST(SmallBitVector, copy_is_independent) {
  strobe::SmallBitVector<64> original(130);
  original.set(2);
  original.set(129);

  auto copy = original;
  EXPECT_EQ(copy, original);
  EXPECT_NE(copy.data(), original.data());

  copy.reset(2);
  EXPECT_TRUE(original.test(2));
}

TEST(SmallBitVector, move_handles_inline_and_heap_storage) {
  strobe::SmallBitVector<64> inlineSource(64);
  inlineSource.set(63);
  strobe::SmallBitVector<64> inlineMoved(std::move(inlineSource));

  EXPECT_TRUE(inlineSource.empty());
  EXPECT_TRUE(inlineMoved.using_inline_storage());
  EXPECT_TRUE(inlineMoved.test(63));

  strobe::SmallBitVector<64> heapSource(130);
  heapSource.set(129);
  const uint64_t *allocation = heapSource.data();

  strobe::SmallBitVector<64> heapMoved(std::move(heapSource));
  EXPECT_TRUE(heapSource.empty());
  EXPECT_EQ(heapMoved.data(), allocation);
  EXPECT_TRUE(heapMoved.test(129));
}

TEST(SmallBitVector, iterates_set_indices_across_words) {
  strobe::SmallBitVector<64> bits(130);
  bits.set(129);
  bits.set(0);
  bits.set(64);

  std::vector<size_t> indices;
  bits.for_each_set_bit([&](size_t index) { indices.push_back(index); });

  EXPECT_EQ(indices, (std::vector<size_t>{0, 64, 129}));
}

TEST(SmallBitVector, zero_inline_capacity_can_grow) {
  strobe::SmallBitVector<0> bits;

  EXPECT_TRUE(bits.empty());
  EXPECT_TRUE(bits.using_inline_storage());

  bits.resize(1);
  EXPECT_FALSE(bits.using_inline_storage());
  EXPECT_FALSE(bits.test(0));

  bits.set(0);
  EXPECT_TRUE(bits.all());
}
