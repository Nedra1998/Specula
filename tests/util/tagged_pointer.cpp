#include <catch2/catch_test_macros.hpp>
#include <specula/specula.hpp>
#include <specula/util/tagged_pointer.hpp>

using namespace specula;

template <int n> struct IntType {
  SPECULA_CPU_GPU int func() { return n; }
  SPECULA_CPU_GPU int cfunc() const { return n; }
};

struct Handle
    : public TaggedPointer<IntType<0>, IntType<1>, IntType<2>, IntType<3>, IntType<4>, IntType<5>,
                           IntType<6>, IntType<7>, IntType<8>, IntType<9>, IntType<10>, IntType<11>,
                           IntType<12>, IntType<13>, IntType<14>, IntType<15>> {
  using TaggedPointer::TaggedPointer;

  int func() {
    auto f = [&](auto ptr) { return ptr->func(); };
    return dispatch_cpu(f);
  }

  int cfunc() const {
    auto f = [&](auto ptr) { return ptr->cfunc(); };
    return dispatch_cpu(f);
  }
};

TEST_CASE("TaggedPointer", "[util]") {
  SECTION("Basics") {
    CHECK(Handle().ptr() == nullptr);

    CHECK(Handle::max_tag() == 16);
    CHECK(Handle::num_tags() == 17);

    IntType<0> it0;
    IntType<1> it1;
    IntType<2> it2;
    IntType<3> it3;
    IntType<4> it4;
    IntType<5> it5;
    IntType<6> it6;
    IntType<7> it7;
    IntType<8> it8;
    IntType<9> it9;
    IntType<10> it10;
    IntType<11> it11;
    IntType<12> it12;
    IntType<13> it13;
    IntType<14> it14;
    IntType<15> it15;

    CHECK(Handle(&it0).is<IntType<0>>());
    CHECK(Handle(&it1).is<IntType<1>>());
    CHECK(Handle(&it2).is<IntType<2>>());
    CHECK(Handle(&it3).is<IntType<3>>());
    CHECK(Handle(&it4).is<IntType<4>>());
    CHECK(Handle(&it5).is<IntType<5>>());
    CHECK(Handle(&it6).is<IntType<6>>());
    CHECK(Handle(&it7).is<IntType<7>>());
    CHECK(Handle(&it8).is<IntType<8>>());
    CHECK(Handle(&it9).is<IntType<9>>());
    CHECK(Handle(&it10).is<IntType<10>>());
    CHECK(Handle(&it11).is<IntType<11>>());
    CHECK(Handle(&it12).is<IntType<12>>());
    CHECK(Handle(&it13).is<IntType<13>>());
    CHECK(Handle(&it14).is<IntType<14>>());
    CHECK(Handle(&it15).is<IntType<15>>());

    CHECK(!Handle(&it0).is<IntType<1>>());
    CHECK(!Handle(&it1).is<IntType<2>>());
    CHECK(!Handle(&it2).is<IntType<3>>());
    CHECK(!Handle(&it3).is<IntType<4>>());
    CHECK(!Handle(&it4).is<IntType<5>>());
    CHECK(!Handle(&it5).is<IntType<6>>());
    CHECK(!Handle(&it6).is<IntType<7>>());
    CHECK(!Handle(&it7).is<IntType<8>>());
    CHECK(!Handle(&it8).is<IntType<9>>());
    CHECK(!Handle(&it9).is<IntType<10>>());
    CHECK(!Handle(&it10).is<IntType<11>>());
    CHECK(!Handle(&it11).is<IntType<12>>());
    CHECK(!Handle(&it12).is<IntType<13>>());
    CHECK(!Handle(&it13).is<IntType<14>>());
    CHECK(!Handle(&it14).is<IntType<15>>());
    CHECK(!Handle(&it15).is<IntType<0>>());

    CHECK(Handle(nullptr).tag() == 0);
    CHECK(Handle(&it0).tag() == 1);
    CHECK(Handle(&it1).tag() == 2);
    CHECK(Handle(&it2).tag() == 3);
    CHECK(Handle(&it3).tag() == 4);
    CHECK(Handle(&it4).tag() == 5);
    CHECK(Handle(&it5).tag() == 6);
    CHECK(Handle(&it6).tag() == 7);
    CHECK(Handle(&it7).tag() == 8);
    CHECK(Handle(&it8).tag() == 9);
    CHECK(Handle(&it9).tag() == 10);
    CHECK(Handle(&it10).tag() == 11);
    CHECK(Handle(&it11).tag() == 12);
    CHECK(Handle(&it12).tag() == 13);
    CHECK(Handle(&it13).tag() == 14);
    CHECK(Handle(&it14).tag() == 15);
    CHECK(Handle(&it15).tag() == 16);

    CHECK(Handle::type_index<decltype(it0)>() == 1);
    CHECK(Handle::type_index<decltype(it1)>() == 2);
    CHECK(Handle::type_index<decltype(it2)>() == 3);
    CHECK(Handle::type_index<decltype(it3)>() == 4);
    CHECK(Handle::type_index<decltype(it4)>() == 5);
    CHECK(Handle::type_index<decltype(it5)>() == 6);
    CHECK(Handle::type_index<decltype(it6)>() == 7);
    CHECK(Handle::type_index<decltype(it7)>() == 8);
    CHECK(Handle::type_index<decltype(it8)>() == 9);
    CHECK(Handle::type_index<decltype(it9)>() == 10);
    CHECK(Handle::type_index<decltype(it10)>() == 11);
    CHECK(Handle::type_index<decltype(it11)>() == 12);
    CHECK(Handle::type_index<decltype(it12)>() == 13);
    CHECK(Handle::type_index<decltype(it13)>() == 14);
    CHECK(Handle::type_index<decltype(it14)>() == 15);
    CHECK(Handle::type_index<decltype(it15)>() == 16);

    CHECK(Handle(&it0).cast_or_nullptr<IntType<0>>() == &it0);
    CHECK(Handle(&it0).cast_or_nullptr<IntType<1>>() == nullptr);
    CHECK(Handle(&it7).cast_or_nullptr<IntType<7>>() == &it7);
    CHECK(Handle(&it7).cast_or_nullptr<IntType<0>>() == nullptr);
    CHECK(Handle(&it7).cast_or_nullptr<IntType<8>>() == nullptr);
  }

  SECTION("Dispatch") {
    IntType<0> it0;
    Handle h0(&it0);
    REQUIRE(it0.func() == 0);
    CHECK(h0.func() == 0);
    REQUIRE(it0.cfunc() == 0);
    CHECK(h0.cfunc() == 0);

    IntType<1> it1;
    Handle h1(&it1);
    REQUIRE(it1.func() == 1);
    CHECK(h1.func() == 1);
    REQUIRE(it1.cfunc() == 1);
    CHECK(h1.cfunc() == 1);

    IntType<2> it2;
    Handle h2(&it2);
    REQUIRE(it2.func() == 2);
    CHECK(h2.func() == 2);
    REQUIRE(it2.cfunc() == 2);
    CHECK(h2.cfunc() == 2);

    IntType<3> it3;
    Handle h3(&it3);
    REQUIRE(it3.func() == 3);
    CHECK(h3.func() == 3);
    REQUIRE(it3.cfunc() == 3);
    CHECK(h3.cfunc() == 3);

    IntType<4> it4;
    Handle h4(&it4);
    REQUIRE(it4.func() == 4);
    CHECK(h4.func() == 4);
    REQUIRE(it4.cfunc() == 4);
    CHECK(h4.cfunc() == 4);

    IntType<5> it5;
    Handle h5(&it5);
    REQUIRE(it5.func() == 5);
    CHECK(h5.func() == 5);
    REQUIRE(it5.cfunc() == 5);
    CHECK(h5.cfunc() == 5);

    IntType<6> it6;
    Handle h6(&it6);
    REQUIRE(it6.func() == 6);
    CHECK(h6.func() == 6);
    REQUIRE(it6.cfunc() == 6);
    CHECK(h6.cfunc() == 6);

    IntType<7> it7;
    Handle h7(&it7);
    REQUIRE(it7.func() == 7);
    CHECK(h7.func() == 7);
    REQUIRE(it7.cfunc() == 7);
    CHECK(h7.cfunc() == 7);

    IntType<8> it8;
    Handle h8(&it8);
    REQUIRE(it8.func() == 8);
    CHECK(h8.func() == 8);
    REQUIRE(it8.cfunc() == 8);
    CHECK(h8.cfunc() == 8);

    IntType<9> it9;
    Handle h9(&it9);
    REQUIRE(it9.func() == 9);
    CHECK(h9.func() == 9);
    REQUIRE(it9.cfunc() == 9);
    CHECK(h9.cfunc() == 9);

    IntType<10> it10;
    Handle h10(&it10);
    REQUIRE(it10.func() == 10);
    CHECK(h10.func() == 10);
    REQUIRE(it10.cfunc() == 10);
    CHECK(h10.cfunc() == 10);

    IntType<11> it11;
    Handle h11(&it11);
    REQUIRE(it11.func() == 11);
    CHECK(h11.func() == 11);
    REQUIRE(it11.cfunc() == 11);
    CHECK(h11.cfunc() == 11);

    IntType<12> it12;
    Handle h12(&it12);
    REQUIRE(it12.func() == 12);
    CHECK(h12.func() == 12);
    REQUIRE(it12.cfunc() == 12);
    CHECK(h12.cfunc() == 12);

    IntType<13> it13;
    Handle h13(&it13);
    REQUIRE(it13.func() == 13);
    CHECK(h13.func() == 13);
    REQUIRE(it13.cfunc() == 13);
    CHECK(h13.cfunc() == 13);

    IntType<14> it14;
    Handle h14(&it14);
    REQUIRE(it14.func() == 14);
    CHECK(h14.func() == 14);
    REQUIRE(it14.cfunc() == 14);
    CHECK(h14.cfunc() == 14);

    IntType<15> it15;
    Handle h15(&it15);
    REQUIRE(it15.func() == 15);
    CHECK(h15.func() == 15);
    REQUIRE(it15.cfunc() == 15);
    CHECK(h15.cfunc() == 15);
  }
}
