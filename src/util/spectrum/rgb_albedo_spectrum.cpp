#include "specula/util/spectrum/rgb_albedo_spectrum.hpp"

#include "specula/util/colorspace.hpp"

SPECULA_CPU_GPU specula::RgbAlbedoSpectrum::RgbAlbedoSpectrum(const RgbColorSpace &cs, Rgb rgb) {
  DASSERT_LE(std::max({rgb.r, rgb.g, rgb.b}), 1);
  DASSERT_GE(std::min({rgb.r, rgb.g, rgb.b}), 0);
  rsp = cs.to_rgb_coeffs(rgb);
}
