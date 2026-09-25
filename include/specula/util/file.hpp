#ifndef SPECULA_UTIL_FILE_HPP
#define SPECULA_UTIL_FILE_HPP

#include <cstddef>
#include <filesystem>
#include <vector>

#include "specula/types.hpp"

namespace specula {
  // TODO: Implement the remaining file handler methods if they are necessary

  std::vector<std::byte> read_file_contents(const std::filesystem::path &filename);
  std::vector<Float> read_float_file(const std::filesystem::path &filename);
} // namespace specula

#endif // SPECULA_UTIL_FILE_HPP
