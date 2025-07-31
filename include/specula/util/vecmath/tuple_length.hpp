#ifndef INCLUDE_VECMATH_TUPLE_LENGTH_HPP_
#define INCLUDE_VECMATH_TUPLE_LENGTH_HPP_

#include "specula/specula.hpp"
#include "specula/util/math/interval.hpp"

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

#endif // INCLUDE_VECMATH_TUPLE_LENGTH_HPP_
