#include <gtest/gtest.h>
#include "containers.h"

// Demonstrate some basic assertions.
TEST(darray_basics, darray_basics1) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);
  darray_destroy(&arr);
}


// Demonstrate some basic assertions.
TEST(darray_basics, darray_basics2) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);

  unsigned int i = 0;
  darray_push_back(&arr, &i);

  EXPECT_EQ(*(unsigned int*)darray_get_back(&arr), 0);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 0), 0);
  EXPECT_EQ(darray_size(&arr), 1);

  darray_pop_back(&arr);

  EXPECT_EQ(darray_size(&arr), 0);

  darray_destroy(&arr);
}

// Demonstrate some basic assertions.
TEST(darray_basics, darray_basics3) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);

  *(unsigned int*)darray_emplace_back(&arr) = 1;

  EXPECT_EQ(*(unsigned int*)darray_get_back(&arr), 1);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 0), 1);
  EXPECT_EQ(darray_size(&arr), 1);

  darray_pop_back(&arr);

  EXPECT_EQ(darray_size(&arr), 0);

  darray_destroy(&arr);
}


// Demonstrate some basic assertions.
TEST(darray_basics, darray_basics4) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);

  *(unsigned int*)darray_emplace_back(&arr) = 1;
  *(unsigned int*)darray_emplace_back(&arr) = 2;
  *(unsigned int*)darray_emplace_back(&arr) = 3;
  *(unsigned int*)darray_emplace_back(&arr) = 4;

  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 0), 1);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 1), 2);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 2), 3);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 3), 4);
  EXPECT_EQ(darray_size(&arr), 4);

  darray_destroy(&arr);
}


// Demonstrate some basic assertions.
TEST(darray_basics, darray_basics5) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);

  darray_emplace_back(&arr);
  darray_emplace_back(&arr);
  darray_emplace_back(&arr);
  darray_emplace_back(&arr);

  unsigned int i = 1;
  darray_set(&arr, 0, &i);
  i = 2;
  darray_set(&arr, 1, &i);
  i = 3;
  darray_set(&arr, 2, &i);
  i = 4;
  darray_set(&arr, 3, &i);

  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 0), 1);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 1), 2);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 2), 3);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 3), 4);
  EXPECT_EQ(darray_size(&arr), 4);

  darray_destroy(&arr);
}

// Demonstrate some basic assertions.
TEST(darray_basics, darray_basics6) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);

  darray_emplace_back(&arr);
  darray_emplace_back(&arr);
  darray_emplace_back(&arr);
  darray_emplace_back(&arr);

  *(unsigned int*)darray_get(&arr, 0) = 1;
  *(unsigned int*)darray_get(&arr, 1) = 2;
  *(unsigned int*)darray_get(&arr, 2) = 3;
  *(unsigned int*)darray_get(&arr, 3) = 4;

  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 0), 1);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 1), 2);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 2), 3);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 3), 4);
  EXPECT_EQ(darray_size(&arr), 4);

  darray_destroy(&arr);
}

// Demonstrate some basic assertions.
TEST(darray_iterator, darray_iterator1) {
  darray arr;
  darray_create(&arr, sizeof(int), 16);

  darray_emplace_back(&arr);
  darray_emplace_back(&arr);
  darray_emplace_back(&arr);
  darray_emplace_back(&arr);

  *(unsigned int*)darray_get(&arr, 0) = 1;
  *(unsigned int*)darray_get(&arr, 1) = 2;
  *(unsigned int*)darray_get(&arr, 2) = 3;
  *(unsigned int*)darray_get(&arr, 3) = 4;

  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 0), 1);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 1), 2);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 2), 3);
  EXPECT_EQ(*(unsigned int*)darray_get(&arr, 3), 4);
  EXPECT_EQ(darray_size(&arr), 4);

  darray_iterator it = darray_iterate(&arr);
  EXPECT_TRUE(darray_iterator_has_next(&it));
  EXPECT_EQ(*(unsigned int*)darray_iterator_next(&it), 1);
  EXPECT_TRUE(darray_iterator_has_next(&it));
  EXPECT_EQ(*(unsigned int*)darray_iterator_next(&it), 2);
  EXPECT_TRUE(darray_iterator_has_next(&it));
  EXPECT_EQ(*(unsigned int*)darray_iterator_next(&it), 3);
  EXPECT_TRUE(darray_iterator_has_next(&it));
  EXPECT_EQ(*(unsigned int*)darray_iterator_next(&it), 4);
  EXPECT_FALSE(darray_iterator_has_next(&it));

  darray_destroy(&arr);
}
