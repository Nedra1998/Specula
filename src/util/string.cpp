#include "util/string.hpp"

std::vector<std::string> specula::split_strings_from_whitespace(std::string_view str) {
  std::vector<std::string> ret;

  std::string_view::iterator start = str.begin();
  do {
    while (start != str.end() && isspace(*start)) {
      ++start;
    }

    auto end = start;
    while (end != str.end() && !isspace(*end)) {
      ++end;
    }
    ret.push_back(std::string(start, end));
    start = end;
  } while (start != str.end());

  return ret;
}
