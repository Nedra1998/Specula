#include "util/colorspace.hpp"

#include <iterator>

#include "util/check.hpp"
#include "util/color/rgb.hpp"
#include "util/color/rgb_sigmoid_polynomial.hpp"
#include "util/color/rgb_spectrum_table.hpp"
#include "util/color/xyz.hpp"
#include "util/math/square_matrix.hpp"
#include "util/spectrum/functions.hpp"
#include "util/spectrum/spectra.hpp"

namespace specula {
  SPECULA_CONST RgbColorSpace *SRGB;
  SPECULA_CONST RgbColorSpace *DCI_P3;
  SPECULA_CONST RgbColorSpace *REC_2020;
  SPECULA_CONST RgbColorSpace *ACES2065_1;
} // namespace specula

const specula::RgbColorSpace *specula::RgbColorSpace::SRGB;
const specula::RgbColorSpace *specula::RgbColorSpace::DCI_P3;
const specula::RgbColorSpace *specula::RgbColorSpace::REC_2020;
const specula::RgbColorSpace *specula::RgbColorSpace::ACES2065_1;

specula::RgbColorSpace::RgbColorSpace(Point2f r, Point2f g, Point2f b, Spectrum illuminant,
                                      const RgbToSpectrumTable *rgb_to_spectrum_table,
                                      Allocator alloc)
    : r(r), g(g), b(b), illuminant(illuminant), rgb_to_spectrum_table(rgb_to_spectrum_table) {
  Xyz W = spectrum_to_xyz(illuminant);
  w = W.xy();
  Xyz R = Xyz::from_xyY(r), G = Xyz::from_xyY(g), B = Xyz::from_xyY(b);

  SquareMatrix<3> rgb(R.x, G.x, B.x, R.y, G.y, B.y, R.z, G.z, B.z);
  Xyz C = inverse_or_exit(rgb) * W;
  xyz_from_rgb = rgb * SquareMatrix<3>::diag(C[0], C[1], C[2]);
  rgb_from_xyz = inverse_or_exit(xyz_from_rgb);
}

void specula::RgbColorSpace::init(Allocator alloc) {
  // Rec. ITU-R BT.709.3
  SRGB = alloc.new_object<RgbColorSpace>(Point2f(.64, .33), Point2f(0.3, 0.6), Point2f(0.15, 0.06),
                                         get_named_spectrum("stdillum-D65"),
                                         RgbToSpectrumTable::SRGB, alloc);

  // P3-D65 (display)
  DCI_P3 = alloc.new_object<RgbColorSpace>(Point2f(.68, .32), Point2f(.265, .690),
                                           Point2f(.15, .06), get_named_spectrum("stdillum-D65"),
                                           RgbToSpectrumTable::DCI_P3, alloc);

  // ITU-R Rec BT.2020
  REC_2020 = alloc.new_object<RgbColorSpace>(
      Point2f(.708, .292), Point2f(.170, .797), Point2f(.131, .046),
      get_named_spectrum("stdillum-D65"), RgbToSpectrumTable::REC_2020, alloc);

  ACES2065_1 = alloc.new_object<RgbColorSpace>(
      Point2f(.7347, .2653), Point2f(0., 1.), Point2f(.0001, -.077),
      get_named_spectrum("illum-acesD60"), RgbToSpectrumTable::ACES2065_1, alloc);

#ifdef SPECULA_BUILD_GPU_RENDERER
  // TODO: If doing a GPU build copy spectrum to the GPU
  static_assert("Not yet implemented!");
#endif
}

const specula::RgbColorSpace *specula::RgbColorSpace::get_named(std::string name) {
  std::string lower_name;
  std::transform(name.begin(), name.end(), std::back_inserter(lower_name), ::tolower);
  if (name == "aces2065-1")
    return ACES2065_1;
  else if (name == "rec2020")
    return REC_2020;
  else if (name == "dci-p3")
    return DCI_P3;
  else if (name == "srgb")
    return SRGB;
  else
    return nullptr;
}

const specula::RgbColorSpace *specula::RgbColorSpace::lookup(Point2f r, Point2f g, Point2f b,
                                                             Point2f w) {
  auto close_enough = [](const Point2f &a, const Point2f &b) {
    return ((a.x == b.x || std::abs((a.x - b.x) / b.x) < 1e-3) &&
            (a.y == b.y || std::abs((a.y - b.y) / b.y) < 1e-3));
  };

  for (const RgbColorSpace *cs : {ACES2065_1, DCI_P3, REC_2020, SRGB}) {
    if (close_enough(r, cs->r) && close_enough(g, cs->g) && close_enough(b, cs->b) &&
        close_enough(w, cs->w)) {
      return cs;
    }
  }
  return nullptr;
}

specula::RgbSigmoidPolynomial specula::RgbColorSpace::to_rgb_coeffs(Rgb rgb) const {
  DASSERT(rgb.r >= 0 && rgb.g >= 0 && rgb.b >= 0);
  return (*rgb_to_spectrum_table)(clamp_zero(rgb));
}

specula::SquareMatrix<3> specula::convert_rgb_color_space(const RgbColorSpace &from,
                                                          const RgbColorSpace &to) {
  if (from == to)
    return {};
  return to.rgb_from_xyz * from.xyz_from_rgb;
}
