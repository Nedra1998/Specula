#ifndef INCLUDE_IMAGE_PIXEL_FORMAT_HPP_
#define INCLUDE_IMAGE_PIXEL_FORMAT_HPP_

#include <string_view>

#include "specula.hpp"

namespace specula {
  enum class PixelFormat { U256, Half, Float };

  SPECULA_CPU_GPU inline bool is_8bit(PixelFormat format) { return format == PixelFormat::U256; }
  SPECULA_CPU_GPU inline bool is_16bit(PixelFormat format) { return format == PixelFormat::Half; }
  SPECULA_CPU_GPU inline bool is_32bit(PixelFormat format) { return format == PixelFormat::Float; }

  SPECULA_CPU_GPU int texel_bytes(PixelFormat format);
} // namespace specula

template <> struct fmt::formatter<specula::PixelFormat> : formatter<std::string_view> {
  auto format(const specula::PixelFormat &v, format_context &ctx) const -> format_context::iterator;
};

#endif // INCLUDE_IMAGE_PIXEL_FORMAT_HPP_
