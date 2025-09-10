#ifndef INCLUDE_COLOR_RGB_SPECTRUM_TABLE_HPP_
#define INCLUDE_COLOR_RGB_SPECTRUM_TABLE_HPP_

#include <cstddef>

#include "specula/specula.hpp"
#include "specula/util/color/rgb.hpp"
#include "specula/util/color/rgb_sigmoid_polynomial.hpp"

namespace specula {
  class RgbToSpectrumTable {
  public:
    static constexpr int RES = 64;

    using CoefficientArray = float[3][RES][RES][RES][3];

    RgbToSpectrumTable(const float *z_nodes, const CoefficientArray *coeffs)
        : z_nodes(z_nodes), coeffs(coeffs) {}

    SPECULA_CPU_GPU RgbSigmoidPolynomial operator()(Rgb rgb) const;

    static void init(Allocator &alloc);

    static const RgbToSpectrumTable *SRGB;
    static const RgbToSpectrumTable *DCI_P3;
    static const RgbToSpectrumTable *REC_2020;
    static const RgbToSpectrumTable *ACES2065_1;

  private:
    const float *z_nodes;
    const CoefficientArray *coeffs;

    friend struct fmt::formatter<RgbSigmoidPolynomial>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::RgbToSpectrumTable> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  inline auto format(const specula::RgbToSpectrumTable &v, FormatContext &ctx) const {
    std::string id;
    if (&v == specula::RgbToSpectrumTable::SRGB) {
      id = "(sRGB) ";
    } else if (&v == specula::RgbToSpectrumTable::DCI_P3) {
      id = "(DCI_P3) ";
    } else if (&v == specula::RgbToSpectrumTable::REC_2020) {
      id = "(Rec2020) ";
    } else if (&v == specula::RgbToSpectrumTable::ACES2065_1) {
      id = "(ACES2065_1) ";
    }
    return format_to(ctx.out(), "[ RgbToSpectrumTable res={} {}]", specula::RgbToSpectrumTable::RES,
                     id);
  }
};

#endif // INCLUDE_COLOR_RGB_SPECTRUM_TABLE_HPP_
