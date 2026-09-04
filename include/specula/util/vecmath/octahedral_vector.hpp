#ifndef SPECULA_UTIL_VECMATH_OCTAHEDRAL_VECTOR_HPP
#define SPECULA_UTIL_VECMATH_OCTAHEDRAL_VECTOR_HPP

#include <cmath>
#include <cstdint>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/math.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  class OctahedralVector {
  public:
    OctahedralVector() = default;

    SPECULA_CPU_GPU OctahedralVector(Vector3f v) {
      v /= std::abs(v.x) + std::abs(v.y) + std::abs(v.z);
      if (v.z >= 0) {
        x = encode(v.x);
        y = encode(v.y);
      } else {
        x = encode((1 - std::abs(v.y)) * sign(v.x));
        y = encode((1 - std::abs(v.x)) * sign(v.y));
      }
    }

    SPECULA_CPU_GPU explicit operator Vector3f() const {
      Vector3f v;

      v.x = -1 + 2 * (static_cast<Float>(x) / 65535.f);
      v.y = -1 + 2 * (static_cast<Float>(y) / 65535.f);
      v.z = 1 - (std::abs(v.x) + std::abs(v.y));

      if (v.z < 0) {
        Float xo = v.x;
        v.x = (1 - std::abs(v.y)) * sign(xo);
        v.y = (1 - std::abs(xo)) * sign(v.y);
      }

      return normalize(v);
    }

  private:
    SPECULA_CPU_GPU static Float sign(Float v) { return std::copysign(1.f, v); }
    SPECULA_CPU_GPU static uint16_t encode(Float f) {
      return static_cast<uint16_t>(pstd::round(clamp((f + 1) / 2, 0, 1) * 65535.f));
    }

    uint16_t x, y;

    friend struct fmt::formatter<OctahedralVector>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::OctahedralVector> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::OctahedralVector &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ OctahedralVector x={} y={} ]", v.x, v.y);
  }
};

#endif // SPECULA_UTIL_VECMATH_OCTAHEDRAL_VECTOR_HPP
