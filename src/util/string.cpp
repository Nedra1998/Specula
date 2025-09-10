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

bool specula::atof(std::string_view str, float *ptr) {
  try {
    *ptr = std::stof(std::string(str.begin(), str.end()));
  } catch (...) {
    return false;
  }
  return true;
}

bool specula::atof(std::string_view str, double *ptr) {
  try {
    *ptr = std::stod(std::string(str.begin(), str.end()));
  } catch (...) {
    return false;
  }
  return true;
}
