#ifndef INCLUDE_UTIL_STRING_HPP_
#define INCLUDE_UTIL_STRING_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace specula {
  std::vector<std::string> split_strings_from_whitespace(std::string_view str);
}

#endif // INCLUDE_UTIL_STRING_HPP_
