#ifndef SPECULA_UTIL_VECMATH_HPP
#define SPECULA_UTIL_VECMATH_HPP

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <specula/specula.hpp>
#include <specula/util/check.hpp>
#include <specula/util/pstd.hpp>

#include "specula/util/vecmath/tuple2.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  namespace {
    template <typename T> struct TupleLength {
      using type = Float;
    };

    template <> struct TupleLength<double> {
      using type = double;
    };

    template <> struct TupleLength<long double> {
      using type = long double;
    };

  } // namespace

} // namespace specula

#endif // SPECULA_UTIL_VECMATH_HPP
