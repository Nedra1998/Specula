#include "util/color/rgb_spectrum_table.hpp"

#include "util/color/rgb_sigmoid_polynomial.hpp"
#include "util/math.hpp"
#include "util/pstd/array.hpp"

namespace specula {
  extern const int sRGBToSpectrumTable_Res;
  extern const float sRGBToSpectrumTable_Scale[64];
  extern const RgbToSpectrumTable::CoefficientArray sRGBToSpectrumTable_Data;

  const RgbToSpectrumTable *RgbToSpectrumTable::sRGB;

  extern const int DCI_P3ToSpectrumTable_Res;
  extern const float DCI_P3ToSpectrumTable_Scale[64];
  extern const RgbToSpectrumTable::CoefficientArray DCI_P3ToSpectrumTable_Data;

  const RgbToSpectrumTable *RgbToSpectrumTable::DCI_P3;

  extern const int REC2020ToSpectrumTable_Res;
  extern const float REC2020ToSpectrumTable_Scale[64];
  extern const RgbToSpectrumTable::CoefficientArray REC2020ToSpectrumTable_Data;

  const RgbToSpectrumTable *RgbToSpectrumTable::Rec2020;

  extern const int ACES2065_1ToSpectrumTable_Res;
  extern const float ACES2065_1ToSpectrumTable_Scale[64];
  extern const RgbToSpectrumTable::CoefficientArray ACES2065_1ToSpectrumTable_Data;

  const RgbToSpectrumTable *RgbToSpectrumTable::ACES2065_1;
} // namespace specula

SPECULA_CPU_GPU specula::RgbSigmoidPolynomial
specula::RgbToSpectrumTable::operator()(specula::Rgb rgb) const {
  DASSERT(rgb[0] >= 0.0f && rgb[1] >= 0.0f && rgb[2] >= 0.0f && rgb[0] <= 1.0f && rgb[1] <= 1.0f &&
          rgb[2] <= 1.0f);

  if (rgb[0] == rgb[1] && rgb[1] == rgb[2]) {
    return RgbSigmoidPolynomial(0.0f, 0.0f, (rgb[0] - 0.5f) / std::sqrt(rgb[0] * (1 - rgb[0])));
  }

  int maxc = (rgb[0] > rgb[1]) ? ((rgb[0] > rgb[2]) ? 0 : 2) : ((rgb[1] > rgb[2]) ? 1 : 2);
  float z = rgb[maxc];
  float x = rgb[(maxc + 1) % 3] * (RES - 1) / z;
  float y = rgb[(maxc + 2) % 3] * (RES - 1) / z;

  int xi = std::min((int)x, RES - 2);
  int yi = std::min((int)y, RES - 2);
  int zi = find_interval(RES, [&](int i) { return z_nodes[i] < z; });

  Float dx = x - xi, dy = y - yi, dz = (z - z_nodes[zi]) / (z_nodes[zi + 1] - z_nodes[zi]);

  pstd::array<Float, 3> c;
  for (int i = 0; i < 3; ++i) {
    auto co = [&](int dx, int dy, int dz) { return (*coeffs)[maxc][zi + dz][yi + dy][xi + dx][i]; };

    c[i] =
        lerp(dz, lerp(dy, lerp(dx, co(0, 0, 0), co(1, 0, 0)), lerp(dx, co(0, 1, 0), co(1, 1, 0))),
             lerp(dy, lerp(dx, co(0, 0, 1), co(1, 0, 1)), lerp(dx, co(0, 1, 1), co(1, 1, 1))));
  }

  return RgbSigmoidPolynomial(c[0], c[1], c[2]);
}

void specula::RgbToSpectrumTable::init(Allocator &alloc) {
#if defined(SPECULA_BUILD_GPU_RENDERER)
  // TODO: Implement GPU initialization
#endif
  // sRGB = alloc.new_object<RgbToSpectrumTable>(sRGBToSpectrumTable_Scale,
  // &sRGBToSpectrumTable_Data); DCI_P3 =
  // alloc.new_object<RgbToSpectrumTable>(DCI_P3ToSpectrumTable_Scale,
  //                                               &DCI_P3ToSpectrumTable_Data);
  // Rec2020 = alloc.new_object<RgbToSpectrumTable>(REC2020ToSpectrumTable_Scale,
  //                                                &REC2020ToSpectrumTable_Data);
  // ACES2065_1 = alloc.new_object<RgbToSpectrumTable>(ACES2065_1ToSpectrumTable_Scale,
  //                                                   &ACES2065_1ToSpectrumTable_Data);
}
