#include "specula/util/spectrum/rgb_unbounded_spectrum.hpp"

#include "specula/util/colorspace.hpp"

SPECULA_CPU_GPU specula::RgbUnboundedSpectrum::RgbUnboundedSpectrum(const RgbColorSpace &cs,
                                                                    Rgb rgb) {
  Float m = std::max({rgb.r, rgb.g, rgb.b});
  scale = 2 * m;
  rsp = cs.to_rgb_coeffs(scale ? rgb / scale : Rgb(0, 0, 0));
}
