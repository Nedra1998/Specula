/**
 * @file white_balance.hpp
 * @brief Functions for white balancing colors in the XYZ color space.
 *
 * This file provides a function to compute a white balance transformation matrix
 * that adjusts the color temperature of an image from a source white point to a target white point.
 */

#ifndef INCLUDE_COLOR_WHITE_BALANCE_HPP_
#define INCLUDE_COLOR_WHITE_BALANCE_HPP_

#include "specula/util/color/xyz.hpp"
#include "specula/util/math/square_matrix.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  /**
   * @brief LMS to XYZ conversion matrix.
   *
   * This matrix is used to convert colors from the LMS color space to the XYZ color space.
   */
  const SquareMatrix<3> LMS_FROM_XYZ(0.8951, 0.2664, -0.1614, -0.7502, 1.7135, 0.0367, 0.0389,
                                     -0.0685, 1.0296);

  /**
   * @brief XYZ to LMS conversion matrix.
   *
   * This matrix is used to convert colors from the XYZ color space to the LMS color space.
   */
  const SquareMatrix<3> XYZ_FROM_LMS(0.986993, -0.147054, 0.159963, 0.432305, 0.51836, 0.0492912,
                                     -0.00852866, 0.0400428, 0.968487);

  /**
   * @brief Computes a white balance transformation matrix.
   *
   * This function implements the white balencing algorithm called the "Von Kries transform".
   * It computes a transformation matrix that adjusts the color temperature of an image
   * from a source white point to a target white point.
   *
   * @param src_white The chromaticities of the illuminant in the source image.
   * @param target_white The chromaticities of the target color white point.
   * @return A 3x3 transformation matrix that applies the white balancing operation to colors in the
   * XYZ color space.
   */
  inline SquareMatrix<3> white_balance(Point2f src_white, Point2f target_white) {
    Xyz src_xyz = Xyz::from_xyY(src_white), dest_xyz = Xyz::from_xyY(target_white);
    auto src_lms = LMS_FROM_XYZ * src_xyz, dst_lms = LMS_FROM_XYZ * dest_xyz;

    SquareMatrix<3> lms_correct = SquareMatrix<3>::diag(
        dst_lms[0] / src_lms[0], dst_lms[1] / src_lms[1], dst_lms[2] / src_lms[2]);
    return XYZ_FROM_LMS * lms_correct * LMS_FROM_XYZ;
  }
} // namespace specula

#endif // INCLUDE_COLOR_WHITE_BALANCE_HPP_
