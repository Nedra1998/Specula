#ifndef INCLUDE_UTIL_FILE_HPP_
#define INCLUDE_UTIL_FILE_HPP_

#include <string_view>
#include <vector>

#include "specula.hpp"

namespace specula {
  std::vector<Float> read_float_file(std::string_view filename);
}

#endif // INCLUDE_UTIL_FILE_HPP_
