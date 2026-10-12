#include "specula/util/image/wrap_mode.hpp"

#include <magic_enum/magic_enum.hpp>

specula::pstd::optional<specula::WrapMode> specula::parse_wrap_mode(const char *v) {
  auto val = magic_enum::enum_cast<WrapMode>(v, magic_enum::case_insensitive);
  if (val.has_value()) {
    return val.value();
  }
  return {};
}
