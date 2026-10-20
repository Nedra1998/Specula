#include "specula/util/containers/type_pack.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace specula;

template <typename T> struct Set {};

TEST_CASE("TypePack", "[util][containers]") {
  SECTION("Index") {
    using Pack = TypePack<int, float, double>;

    CHECK(IndexOf<int, Pack>::count == 0);
    CHECK(IndexOf<float, Pack>::count == 1);
    CHECK(IndexOf<double, Pack>::count == 2);
  }

  SECTION("HasType") {
    using Pack = TypePack<signed int, float, double>;

    CHECK(HasType<int, Pack>::value);
    CHECK(HasType<float, Pack>::value);
    CHECK(HasType<double, Pack>::value);

    CHECK_FALSE(HasType<char, Pack>::value);
    CHECK_FALSE(HasType<unsigned int, Pack>::value);
  }

  SECTION("TakeRemove") {
    using Pack = TypePack<signed int, float, double>;

    CHECK(std::is_same_v<TypePack<signed int>, typename TakeFirstN<1, Pack>::type>);
    CHECK(std::is_same_v<TypePack<float>,
                         typename TakeFirstN<1, typename RemoveFirstN<1, Pack>::type>::type>);
    CHECK(std::is_same_v<TypePack<double>,
                         typename TakeFirstN<1, typename RemoveFirstN<2, Pack>::type>::type>);
  }

  SECTION("Map") {
    using SetPack = typename MapType<Set, TypePack<signed int, float, double>>::type;

    CHECK(std::is_same_v<TypePack<Set<signed int>>, typename TakeFirstN<1, SetPack>::type>);
    CHECK(std::is_same_v<TypePack<Set<float>>,
                         typename TakeFirstN<1, typename RemoveFirstN<1, SetPack>::type>::type>);
    CHECK(std::is_same_v<TypePack<Set<double>>,
                         typename TakeFirstN<1, typename RemoveFirstN<2, SetPack>::type>::type>);
  }
}
