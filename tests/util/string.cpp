#include "specula/util/string.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace specula;

TEST_CASE("UnicodeString", "[util]") {
  SECTION("Basic normalization") {
    std::u16string nfc16(u"\u0041\u006d\u00e9\u006c\u0069\u0065");
    std::u16string nfd16(u"\u0041\u006d\u0065\u0301\u006c\u0069\u0065");
    CHECK(nfc16 != nfd16);

    std::string nfc8 = utf8_from_utf16(nfc16);
    std::string nfd8 = utf8_from_utf16(nfd16);
    CHECK(nfc8 != nfd8);

    CHECK(nfc8 == normalize_utf8(nfc8));
    CHECK(nfc8 == normalize_utf8(nfd8));
  }
}
