#include "util/image/pixel_format.hpp"

#include <magic_enum/magic_enum.hpp>

#include "specula.hpp"
#include "util/log.hpp"

SPECULA_CPU_GPU int specula::texel_bytes(PixelFormat format) {
  switch (format) {
  case PixelFormat::U256:
    return 1;
  case PixelFormat::Half:
    return 2;
  case PixelFormat::Float:
    return 4;
  default:
    LOG_CRITICAL("Unhandled PixelFormat: {}", magic_enum::enum_name(format));
    return 0;
  }
}

auto fmt::formatter<specula::PixelFormat>::format(const specula::PixelFormat &v,
                                                  format_context &ctx) const
    -> format_context::iterator {
  return formatter<string_view>::format(magic_enum::enum_name(v), ctx);
}
