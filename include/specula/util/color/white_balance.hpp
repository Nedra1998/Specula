#ifndef SPECULA_UTIL_COLOR_WHITE_BALANCE_HPP
#define SPECULA_UTIL_COLOR_WHITE_BALANCE_HPP

// IWYU pragma: private, include "specula/util/color.hpp"

#include "specula/util/color/xyz.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {

  const SquareMatrix<3> LMS_FROM_XYZ(0.8951, 0.2664, -0.1614, -0.7502, 1.7135, 0.0367, 0.0389,
                                     -0.0685, 1.0296);
  const SquareMatrix<3> XYZ_FROM_LMS(0.986993, -0.147054, 0.159963, 0.432305, 0.51836, 0.0492912,
                                     -0.00852866, 0.0400428, 0.968487);

  inline SquareMatrix<3> white_balance(Point2f src_white, Point2f target_white) {
    Xyz src_xyz = Xyz::from_xyY(src_white), dst_xyz = Xyz::from_xyY(target_white);
    auto src_lms = LMS_FROM_XYZ * src_xyz, dst_lms = LMS_FROM_XYZ * dst_xyz;

    SquareMatrix<3> lms_correct = SquareMatrix<3>::diag(
        dst_lms[0] / src_lms[0], dst_lms[1] / src_lms[1], dst_lms[2] / src_lms[2]);
    return XYZ_FROM_LMS * lms_correct * LMS_FROM_XYZ;
  }
} // namespace specula

#endif // SPECULA_UTIL_COLOR_WHITE_BALANCE_HPP
