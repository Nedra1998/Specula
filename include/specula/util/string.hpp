#ifndef SPECULA_UTIL_STRING_HPP
#define SPECULA_UTIL_STRING_HPP

#include <string>
#include <string_view>
#include <vector>

namespace specula {
  bool atoi(std::string_view str, int *);
  bool atoi(std::string_view str, int64_t *);
  bool atof(std::string_view str, double *);
  bool atof(std::string_view str, float *);

  std::vector<std::string> split_strings_from_whitesapce(std::string_view str);

  std::vector<std::string> split_string(std::string_view str, char ch);
  std::vector<int> split_string_to_ints(std::string_view str, char ch);
  std::vector<int64_t> split_string_to_int64s(std::string_view str, char ch);
  std::vector<float> split_string_to_floats(std::string_view str, char ch);
  std::vector<double> split_string_to_doubles(std::string_view str, char ch);

  std::string utf8_from_utf16(const std::u16string &str);
  std::u16string utf16_from_utf8(const std::string &str);

#ifdef SPECULA_IS_WINDOWS
  std::wstring wstring_from_utf8(const std::string &str);
  std::string utf8_from_wstring(const std::wstring &str);
#endif // SPECULA_IS_WINDOWS

  std::string normalize_utf8(const std::string &str);

  class InternedString {
  public:
    InternedString() = default;
    InternedString(const std::string *str) : str(str) {}
    operator const std::string &() const { return *str; }

    bool operator==(const char *s) const { return *str == s; }
    bool operator==(const std::string &s) const { return *str == s; }
    bool operator!=(const char *s) const { return *str != s; }
    bool operator!=(const std::string &s) const { return *str != s; }
    bool operator<(const char *s) const { return *str < s; }
    bool operator<(const std::string &s) const { return *str < s; }

  private:
    const std::string *str = nullptr;
  };

  struct InternedStringHash {
    size_t operator()(const InternedString &s) const { return std::hash<std::string>()(s); }
  };

  inline auto format_as(const InternedString s) { return std::string(s); }
} // namespace specula

#endif // SPECULA_UTIL_STRING_HPP
