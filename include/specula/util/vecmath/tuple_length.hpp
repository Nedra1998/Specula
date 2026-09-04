#ifndef SPECULA_UTIL_VECMATH_TUPLE_LENGTH_HPP
#define SPECULA_UTIL_VECMATH_TUPLE_LENGTH_HPP

#include "specula/types.hpp"
#include "specula/util/math.hpp"

namespace specula {
  template <typename T> struct TupleLength {
    using type = Float;
  };
  template <> struct TupleLength<double> {
    using type = double;
  };
  template <> struct TupleLength<long double> {
    using type = long double;
  };
  template <> struct TupleLength<Interval> {
    using type = Interval;
  };
} // namespace specula

#endif // SPECULA_UTIL_VECMATH_TUPLE_LENGTH_HPP
