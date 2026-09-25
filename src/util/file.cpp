#include "specula/util/file.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

#include <fmt/std.h>

#include "specula/util/check.hpp"
#include "specula/util/log.hpp"
#include "specula/util/parallel/thread_local.hpp"
#include "specula/util/string.hpp"

std::vector<std::byte> specula::read_file_contents(const std::filesystem::path &filename) {
#ifdef SPECULA_IS_WINDOWS
  std::ifstream ifs(wstring_from_utf8(filename.string()), std::ios::in | std::ios::binary);
#else
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
#endif
  if (!ifs) {
    LOG_CRITICAL("{}: {}", filename, std::strerror(errno));
    std::quick_exit(1);
  }

  ifs.seekg(0, std::ios::end);
  std::streamsize length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::vector<std::byte> data;

  std::vector<uint8_t> buf(length);
  ifs.read(reinterpret_cast<char *>(buf.data()), length);
  ifs.close();

  return data;
}

std::vector<specula::Float> specula::read_float_file(const std::filesystem::path &filename) {
#ifdef SPECULA_IS_WINDOWS
  FILE *f = _wfopen(wstring_from_utf8(filename.string()).c_str(), "rb");
#else
  FILE *f = fopen(filename.string().c_str(), "rb");
#endif

  if (f == nullptr) {
    LOG_ERROR("Unable to open file: {}", filename);
    return {};
  }

  int c;
  bool in_number = false;
  char cur_number[32];
  int cur_number_pos = 0;
  int line_number = 1;

  std::vector<Float> values;
  while ((c = getc(f)) != EOF) {
    if (c == '\n') {
      ++line_number;
    }
    if (in_number) {
      if (cur_number_pos >= (int)sizeof(cur_number)) {
        LOG_CRITICAL("Overflowed buffer for parsing number in file {} at line {}", filename,
                     line_number);
        std::quick_exit(1);
      }

      if ((isdigit(c) != 0) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+') {
        ASSERT_LT(cur_number_pos, sizeof(cur_number));
        cur_number[cur_number_pos++] = c;
      } else {
        cur_number[cur_number_pos++] = '\0';
        Float v;
        if (!atof(cur_number, &v)) {
          LOG_CRITICAL("Unabled to parse float value \"{}\" from {}", cur_number, filename);
          std::quick_exit(1);
        }
        values.push_back(v);
        in_number = false;
        cur_number_pos = 0;
      }
    } else {
      if ((isdigit(c) != 0) || c == '.' || c == '-' || c == '+') {
        in_number = true;
        cur_number[cur_number_pos++] = c;
      } else if (c == '#') {
        while ((c = getc(f)) != '\n' && c != EOF) {
        };
        ++line_number;
      } else if (isspace(c) == 0) {
        LOG_ERROR("Unexpected character \"{}\" found in {} at line {}", c, filename, line_number);
        return {};
      }
    }
  }

  fclose(f);
  return values;
}
