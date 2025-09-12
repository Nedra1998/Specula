#ifndef INCLUDE_IMAGE_WRAP_MODE_HPP_
#define INCLUDE_IMAGE_WRAP_MODE_HPP_

#include "specula.hpp"
#include "util/check.hpp"
#include "util/log.hpp"
#include "util/pstd/array.hpp"
#include "util/vecmath/tuple2.hpp"
namespace specula {
  enum class WrapMode { Black, Clamp, Repeat, OctahedralSphere };

  struct WrapMode2D {
    SPECULA_CPU_GPU WrapMode2D(WrapMode w) : wrap{w, w} {}
    SPECULA_CPU_GPU WrapMode2D(WrapMode x, WrapMode y) : wrap{x, y} {}

    pstd::array<WrapMode, 2> wrap;
  };

  SPECULA_CPU_GPU inline bool remap_pixel_coords(Point2i *pp, Point2i resolution,
                                                 WrapMode2D wrap_mode) {
    Point2i &p = *pp;

    if (wrap_mode.wrap[0] == WrapMode::OctahedralSphere) {
      ASSERT(wrap_mode.wrap[1] == WrapMode::OctahedralSphere);
      if (p[0] < 0) {
        p[0] = -p[0];
        p[1] = resolution[1] - 1 - p[1];
      } else if (p[0] >= resolution[0]) {
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

      switch (wrap_mode.wrap[c]) {
      case WrapMode::Repeat:
        p[c] = mod(p[c], resolution[c]);
        break;
      case WrapMode::Clamp:
        p[c] = clamp(p[c], 0, resolution[c] - 1);
        break;
      case WrapMode::Black:
        return false;
      default:
        LOG_CRITICAL("Unhandled WrapMode");
      }
    }

    return true;
  }
} // namespace specula

#endif // INCLUDE_IMAGE_WRAP_MODE_HPP_
