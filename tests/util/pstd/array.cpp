#include "specula/util/pstd/array.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace specula;

TEST_CASE("Zero-Sized Array", "[util][pstd]") {
  pstd::array<int, 0> zero_array;

  SECTION("Size of zero-sized array") { CHECK(zero_array.size() == 0); }

  SECTION("Begin and end iterators are null") {
    CHECK(zero_array.begin() == nullptr);
    CHECK(zero_array.end() == nullptr);
  }

  SECTION("Equality always true") {
    pstd::array<int, 0> another_zero_array;
    CHECK(zero_array == another_zero_array);
    CHECK_FALSE(zero_array != another_zero_array);
  }
}

TEST_CASE("Fixed-Sized Array", "[util][pstd]") {
  constexpr size_t array_size = 5;
  pstd::array<int, array_size> fixed_array;

  SECTION("Default construction") {
    for (size_t i = 0; i < array_size; ++i) {
      CHECK(fixed_array[i] == 0);
    }
  }

  SECTION("Initializer list construction") {
    pstd::array<int, array_size> initialized_array({1, 2, 3, 4, 5});
    for (size_t i = 0; i < array_size; ++i) {
      CHECK(initialized_array[i] == static_cast<int>(i + 1));
    }
  }

  SECTION("Fill the array") {
    fixed_array.fill(42);
    for (size_t i = 0; i < array_size; ++i) {
      CHECK(fixed_array[i] == 42);
    }
  }

  SECTION("Equality operator") {
    pstd::array<int, array_size> another_array;
    another_array.fill(0);
    CHECK(fixed_array == another_array);
    CHECK_FALSE(fixed_array != another_array);

    another_array[0] = 1;
    CHECK(fixed_array != another_array);
    CHECK_FALSE(fixed_array == another_array);
  }

  SECTION("Iterators") {
    fixed_array.fill(10);
    size_t count = 0;
    for (auto it = fixed_array.begin(); it != fixed_array.end(); ++it, ++count) {
      CHECK(*it == 10);
    }
    CHECK(count == array_size);
  }

  SECTION("Direct data access") {
    fixed_array.fill(99);
    int *data_ptr = fixed_array.data();
    for (size_t i = 0; i < array_size; ++i) {
      CHECK(data_ptr[i] == 99);
    }
  }
}
