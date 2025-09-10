#include "util/file.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

#include "util/check.hpp"
#include "util/log.hpp"
#include "util/string.hpp"

FILE *fopen_read(std::string_view filename) { return fopen(filename.data(), "rb"); }

std::vector<specula::Float> specula::read_float_file(std::string_view filename) {
  FILE *f = fopen_read(filename);
  if (f == nullptr) {
    LOG_ERROR("Unable to open file {}", filename);
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
