#ifndef SPECULA_UTIL_NOISE_HPP
#define SPECULA_UTIL_NOISE_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"

namespace specula {
  template <typename T> class Point3;
  using Point3f = Point3<Float>;
  template <typename T> class Vector3;
  using Vector3f = Vector3<Float>;

  SPECULA_CPU_GPU Float noise(Float x, Float y = 0.5f, Float z = 0.5f);
  SPECULA_CPU_GPU Float noise(Point3f p);
  SPECULA_CPU_GPU Vector3f dnoise(Point3f p);
  SPECULA_CPU_GPU Float fbm(Point3f p, Vector3f dpdx, Vector3f dpdy, Float omega, int octaves);
  SPECULA_CPU_GPU Float turbulence(Point3f p, Vector3f dpdx, Vector3f dpdy, Float omega,
                                   int octaves);
} // namespace specula

#endif // SPECULA_UTIL_NOISE_HPP
