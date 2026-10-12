#ifndef SPECULA_UTIL_IMAGE_PIXEL_FORMAT_HPP
#define SPECULA_UTIL_IMAGE_PIXEL_FORMAT_HPP

#include <cstdint>

#include <fmt/base.h>

#include "specula/macros.hpp"

namespace specula {
  enum class PixelFormat : uint8_t { U256, HALF, FLOAT };

  SPECULA_CPU_GPU inline bool is_8bit(PixelFormat format) { return format == PixelFormat::U256; }
  SPECULA_CPU_GPU inline bool is_16bit(PixelFormat format) { return format == PixelFormat::HALF; }
  SPECULA_CPU_GPU inline bool is_32bit(PixelFormat format) { return format == PixelFormat::FLOAT; }

  SPECULA_CPU_GPU int texel_bytes(PixelFormat format);
} // namespace specula

template <> struct fmt::formatter<specula::PixelFormat> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::PixelFormat &v, FormatContext &ctx) const {
    switch (v) {
    case specula::PixelFormat::U256:
      return format_to(ctx.out(), "U256");
    case specula::PixelFormat::HALF:
      return format_to(ctx.out(), "Half");
    case specula::PixelFormat::FLOAT:
      return format_to(ctx.out(), "Float");
    }
  }
};

#endif // SPECULA_UTIL_IMAGE_PIXEL_FORMAT_HPP
