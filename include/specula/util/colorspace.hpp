#ifndef INCLUDE_UTIL_COLORSPACE_HPP_
#define INCLUDE_UTIL_COLORSPACE_HPP_

#include "specula.hpp"
#include "util/color/rgb.hpp"
#include "util/color/rgb_sigmoid_polynomial.hpp"
#include "util/color/rgb_spectrum_table.hpp"
#include "util/color/xyz.hpp"
#include "util/math/square_matrix.hpp"
#include "util/spectrum/densly_sampled_spectrum.hpp"
#include "util/spectrum/spectrum.hpp"
#include "util/vecmath/tuple2.hpp"

namespace specula {

#ifdef SPECULA_BUILD_GPU_RENDERER
  extern SPECULA_CONST RgbColorSpace *SRGB;
  extern SPECULA_CONST RgbColorSpace *DCI_P3;
  extern SPECULA_CONST RgbColorSpace *REC_2020;
  extern SPECULA_CONST RgbColorSpace *ACES2065_1;
#endif

  class RgbColorSpace {
  public:
    RgbColorSpace(Point2f r, Point2f g, Point2f b, Spectrum illuminant,
                  const RgbToSpectrumTable *rgb_to_spectrum_table, Allocator alloc);

    static void init(Allocator alloc);
    static const RgbColorSpace *get_named(std::string name);
    static const RgbColorSpace *lookup(Point2f r, Point2f g, Point2f b, Point2f w);

    SPECULA_CPU_GPU RgbSigmoidPolynomial to_rgb_coeffs(Rgb rgb) const;

    SPECULA_CPU_GPU Rgb luminanace_vector() const {
      return Rgb(xyz_from_rgb[1][0], xyz_from_rgb[1][1], xyz_from_rgb[1][2]);
    }

    SPECULA_CPU_GPU Rgb to_rgb(Xyz xyz) const { return mul<Rgb>(rgb_from_xyz, xyz); }
    SPECULA_CPU_GPU Xyz to_xyz(Rgb rgb) const { return mul<Xyz>(xyz_from_rgb, rgb); }

    SPECULA_CPU_GPU bool operator==(const RgbColorSpace &cs) const {
      return r == cs.r && g == cs.g && b == cs.b && w == cs.w &&
             rgb_to_spectrum_table == cs.rgb_to_spectrum_table;
    }

    SPECULA_CPU_GPU bool operator!=(const RgbColorSpace &cs) const {
      return r != cs.r || g != cs.g || b != cs.b || w != cs.w ||
             rgb_to_spectrum_table != cs.rgb_to_spectrum_table;
    }

    Point2f r, g, b, w;
    DenslySampledSpectrum illuminant;
    SquareMatrix<3> xyz_from_rgb, rgb_from_xyz;
    static const RgbColorSpace *SRGB, *DCI_P3, *REC_2020, *ACES2065_1;

  private:
    const RgbToSpectrumTable *rgb_to_spectrum_table;
  };

  SquareMatrix<3> convert_rgb_color_space(const RgbColorSpace &from, const RgbColorSpace &to);
} // namespace specula

template <> struct fmt::formatter<specula::RgbColorSpace> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbColorSpace &v, FormatContext &ctx) const {
    return format_to(ctx.out(),
                     "[ RgbColorSpace r={} g={} b={} w={} illuminant={} RgbToXyz={} XyzToRgb={} ]",
                     v.r, v.g, v.b, v.w, v.illuminant, v.rgb_from_xyz, v.xyz_from_rgb);
  }
};

#endif // INCLUDE_UTIL_COLORSPACE_HPP_
