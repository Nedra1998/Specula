#ifndef SPECULA_UTIL_IMAGE_WRAP_MODE_HPP
#define SPECULA_UTIL_IMAGE_WRAP_MODE_HPP

#include <cstdint>

#include "specula/util/log.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  enum class WrapMode : uint8_t { BLACK, CLAMP, REPEAT, OCTAHEDRAL_SPHERE };
}

template <> struct fmt::formatter<specula::WrapMode> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::WrapMode &v, FormatContext &ctx) const {
    switch (v) {
    case specula::WrapMode::BLACK:
      return format_to(ctx.out(), "Black");
    case specula::WrapMode::CLAMP:
      return format_to(ctx.out(), "Clamp");
    case specula::WrapMode::REPEAT:
      return format_to(ctx.out(), "Repeat");
    case specula::WrapMode::OCTAHEDRAL_SPHERE:
      return format_to(ctx.out(), "OctahedralSphere");
    }
  }
};

namespace specula {

  struct WrapMode2D {
    SPECULA_CPU_GPU WrapMode2D(WrapMode w) : wrap{w, w} {}
    SPECULA_CPU_GPU WrapMode2D(WrapMode x, WrapMode y) : wrap{x, y} {}

    pstd::array<WrapMode, 2> wrap;
  };

  pstd::optional<WrapMode> parse_wrap_mode(const char *v);

  SPECULA_CPU_GPU inline bool remap_pixel_coords(Point2i *pp, Point2i resolution,
                                                 WrapMode2D wrapMode) {
    Point2i &p = *pp;

    if (wrapMode.wrap[0] == WrapMode::OCTAHEDRAL_SPHERE) {
      ASSERT(wrapMode.wrap[1] == WrapMode::OCTAHEDRAL_SPHERE);
      if (p[0] < 0) {
        p[0] = -p[0];
        p[1] = resolution[1] - 1 - p[1];
      } else if (p[1] >= resolution[0]) {
        p[0] = 2 * resolution[0] - 1 - p[0];
        p[1] = resolution[1] - 1 - p[1];
      }

      if (p[1] < 0) {
        p[0] = resolution[0] - 1 - p[0];
        p[1] = -p[1];
      } else if (p[1] >= resolution[1]) {
        p[0] = resolution[0] - 1 - p[0];
        p[1] = 2 * resolution[1] - 1 - p[1];
      }

      if (resolution[0] == 1) {
        p[0] = 0;
      }
      if (resolution[1] == 1) {
        p[1] = 0;
      }

      return true;
    }

    for (int c = 0; c < 2; ++c) {
      if (p[c] >= 0 && p[c] < resolution[c]) {
        continue;
      }

      switch (wrapMode.wrap[c]) {
      case WrapMode::BLACK:
        return false;
      case WrapMode::CLAMP:
        p[c] = clamp(p[c], 0, resolution[c] - 1);
        break;
      case WrapMode::REPEAT:
        p[c] = mod(p[c], resolution[c]);
        break;
      default:
        LOG_CRITICAL("Unhandled WrapMode: {}", wrapMode.wrap[c]);
      }
    }
    return true;
  }

} // namespace specula

#endif // SPECULA_UTIL_IMAGE_WRAP_MODE_HPP
