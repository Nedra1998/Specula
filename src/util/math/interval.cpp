#include "specula/util/math/interval.hpp"

#include <numbers>

#ifndef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
const specula::Interval specula::Interval::Pi(3.1415926535897931, 3.1415926535897936);
#  else
const specula::Interval specula::Interval::PI =
    Interval(std::numbers::pi_v<float>, std::numbers::pi_v<float>);
#  endif
#endif
