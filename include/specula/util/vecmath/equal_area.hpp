#ifndef SPECULA_UTIL_VECMATH_EQUAL_AREA_HPP
#define SPECULA_UTIL_VECMATH_EQUAL_AREA_HPP

#include "specula/macros.hpp"
#include "specula/util/vecmath/tuple2.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  SPECULA_CPU_GPU Vector3f equal_area_square_to_sphere(Point2f p);
  SPECULA_CPU_GPU Point2f equal_area_sphere_to_square(Vector3f d);
  SPECULA_CPU_GPU Point2f wrap_equal_area_square(Point2f uv);
} // namespace specula

#endif // SPECULA_UTIL_VECMATH_EQUAL_AREA_HPP
