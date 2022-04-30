#include "base/for_all_of.hpp"

#include <algorithm>
#include <array>
#include <tuple>

#include "gtest/gtest.h"

namespace principia {
namespace base {

class ForAllOfTest : public ::testing::Test {
 protected:
  using AnArray = std::array<float, 3>;
  using APair = std::pair<int*, char>;
  using ATuple = std::tuple<char, double, int>;
};

TEST_F(ForAllOfTest, AnArray) {
  constexpr AnArray halved = []() {
    AnArray array{42.0, 43.0, -41.0};
    for_all_of(array, [](auto& value) { value /= 2; });
    return array;
  }();
  static_assert(std::get<0>(halved) == 21.0);
  static_assert(std::get<1>(halved) == 21.5);
  static_assert(std::get<2>(halved) == -20.5);
}

TEST_F(ForAllOfTest, APair) {
  std::array<int, 1> a;
  constexpr APair incremented = [&a]() {
    APair pair{a.data(), 'y'};
    for_all_of(pair, [](auto& value) { ++value; });
    return pair;
  }();
  static_assert(std::get<0>(incremented) == &a[1]);
  static_assert(std::get<1>(incremented) == 'z');
}

TEST_F(ForAllOfTest, ATuple) {
  constexpr ATuple incremented = []() {
    ATuple tuple{'a', 42.0, 666};
    for_all_of(tuple, [](auto& value) { ++value; });
    return tuple;
  }();
  static_assert(std::get<0>(incremented) == 'b');
  static_assert(std::get<1>(incremented) == 43.0);
  static_assert(std::get<2>(incremented) == 667);
}

}  // namespace base
}  // namespace principia
