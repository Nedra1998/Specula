#include "specula/util/string.hpp"

#include <cstdlib>

#include <utf8/cpp17.h>
#include <utf8proc.h>

#include "specula/util/log.hpp"

bool specula::atoi(std::string_view str, int *ptr) {
  try {
    *ptr = std::stoi(std::string(str.begin(), str.end()));
  } catch (...) {
    return false;
  }
  return true;
}

bool specula::atoi(std::string_view str, int64_t *ptr) {
  try {
    *ptr = std::stoll(std::string(str.begin(), str.end()));
  } catch (...) {
    return false;
  }
  return true;
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

std::vector<std::string> specula::split_strings_from_whitesapce(std::string_view str) {
  std::vector<std::string> ret;

  std::string_view::iterator start = str.begin();
  do {
    while (start != str.end() && (isspace(*start) != 0)) {
      ++start;
    }

    const auto *end = start;
    while (end != str.end() && (isspace(*end) == 0)) {
      ++end;
    }
    ret.emplace_back(start, end);
    start = end;
  } while (start != str.end());

  return ret;
}

std::vector<std::string> specula::split_string(std::string_view str, char ch) {
  std::vector<std::string> strings;
  if (str.empty()) {
    return strings;
  }

  std::string_view::iterator begin = str.begin();
  while (true) {
    std::string_view::iterator end = begin;
    while (end != str.end() && *end != ch) {
      ++end;
    }
    strings.emplace_back(begin, end);
    if (end == str.end()) {
      break;
    }
    begin = end + 1;
  }

  return strings;
}

std::vector<int> specula::split_string_to_ints(std::string_view str, char ch) {
  std::vector<std::string> strs = split_string(str, ch);
  std::vector<int> ints(strs.size());

  for (size_t i = 0; i < strs.size(); ++i) {
    if (!atoi(strs[i], &ints[i])) {
      return {};
    }
  }
  return ints;
}

std::vector<int64_t> specula::split_string_to_int64s(std::string_view str, char ch) {
  std::vector<std::string> strs = split_string(str, ch);
  std::vector<int64_t> ints(strs.size());

  for (size_t i = 0; i < strs.size(); ++i) {
    if (!atoi(strs[i], &ints[i])) {
      return {};
    }
  }
  return ints;
}

std::vector<float> specula::split_string_to_floats(std::string_view str, char ch) {
  std::vector<std::string> strs = split_string(str, ch);
  std::vector<float> floats(strs.size());

  for (size_t i = 0; i < strs.size(); ++i) {
    if (!atof(strs[i], &floats[i])) {
      return {};
    }
  }
  return floats;
}

std::vector<double> specula::split_string_to_doubles(std::string_view str, char ch) {
  std::vector<std::string> strs = split_string(str, ch);
  std::vector<double> floats(strs.size());

  for (size_t i = 0; i < strs.size(); ++i) {
    if (!atof(strs[i], &floats[i])) {
      return {};
    }
  }
  return floats;
}

std::string specula::utf8_from_utf16(const std::u16string &str) { return utf8::utf16to8(str); }

std::u16string specula::utf16_from_utf8(const std::string &str) { return utf8::utf8to16(str); }

#ifdef SPECULA_IS_WINDOWS
std::string wstring_from_u16string(const std::u16string &str) {
  std::wstring ws;
  ws.reserve(str.size());
  for (char16_t c : str) {
    ws.push_back(c);
  }
  return ws;
}

std::u16string u16string_from_wstring(std::wstring str) {
  std::u16string su16;
  su16.reserve(str.size());
  for (wchar_t c : str) {
    su16.push_back(c);
  }
  return su16;
}

std::wstring specula::wstring_from_utf8(const std::string &str) {
  return wstring_from_u16string(utf16_from_utf8(str));
}

std::string specula::utf8_from_wstring(const std::wstring &str) {
  return utf8_from_utf16(u16string_from_wstring(str));
}
#endif

std::string normalize_utf8(const std::string &str) {
  utf8proc_option_t options = UTF8PROC_COMPOSE;

  utf8proc_uint8_t *result = nullptr;
  utf8proc_ssize_t length =
      utf8proc_map((const unsigned char *)str.data(), static_cast<utf8proc_ssize_t>(str.size()),
                   &result, options);
  if (length < 0) {
    LOG_CRITICAL("Unicode normalization error: {}: \"{}\"", utf8proc_errmsg(length), str);
    std::quick_exit(1);
  }
  std::string out = std::string(result, result + length);
  free(result); // NOLINT
  return out;
}
