#ifndef SPECULA_UTIL_SCATTERING_FUNCTIONS_HPP
#define SPECULA_UTIL_SCATTERING_FUNCTIONS_HPP

#include "specula/macros.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/spectrum.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  SPECULA_CPU_GPU inline Vector3f reflect(Vector3f wo, Vector3f n) {
    return -wo + 2 * dot(wo, n) * n;
  }

  SPECULA_CPU_GPU inline bool refract(Vector3f wi, Normal3f n, Float eta, Float *etap,
                                      Vector3f *wt) {
    Float cos_theta_i = dot(n, wi);
    if (cos_theta_i < 0) {
      eta = 1 / eta;
      cos_theta_i = -cos_theta_i;
      n = -n;
    }

    Float sin2_theta_i = std::max<Float>(0, 1 - sqr(cos_theta_i));
    Float sin2_theta_t = sin2_theta_i / sqr(eta);

    if (sin2_theta_t >= 1) {
      return false;
    }

    Float cos_theta_t = std::sqrt(1 - sin2_theta_t);

    *wt = -wi / eta + (cos_theta_i / eta - cos_theta_t) * Vector3f(n);
    if (etap != nullptr) {
      *etap = eta;
    }
    return true;
  }

  SPECULA_CPU_GPU inline Float henyey_greenstein(Float cos_theta, Float g) {
    g = clamp(g, -0.99, 0.99);
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return INV_4PI * (1 - sqr(g)) / (denom * safe_sqrt(denom));
  }

  SPECULA_CPU_GPU inline Float fresnel_dielectric(Float cos_theta_i, Float eta) {
    cos_theta_i = clamp(cos_theta_i, -1, 1);
    if (cos_theta_i < 0) {
      eta = 1 / eta;
      cos_theta_i = -cos_theta_i;
    }

    Float sin2_theta_i = 1 - sqr(cos_theta_i);
    Float sin2_theta_t = sin2_theta_i / sqr(eta);
    if (sin2_theta_t >= 1) {
      return 1.0f;
    }
    Float cos_theta_t = safe_sqrt(1 - sin2_theta_t);

    Float r_parl = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);
    Float r_perp = (cos_theta_i - eta * cos_theta_t) / (cos_theta_i + eta * cos_theta_t);
    return (sqr(r_parl) + sqr(r_perp)) / 2;
  }

  SPECULA_CPU_GPU inline Float fresnel_complex(Float cos_theta_i, pstd::complex<Float> eta) {
    using Complex = pstd::complex<Float>;
    cos_theta_i = clamp(cos_theta_i, 0, 1);

    Float sin2_theta_i = 1 - sqr(cos_theta_i);
    Complex sin2_theta_t = sin2_theta_i / sqr(eta);
    Complex cos_theta_t = pstd::sqrt(1 - sin2_theta_t);

    Complex r_parl = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);
    Complex r_perp = (cos_theta_i - eta * cos_theta_t) / (cos_theta_i + eta * cos_theta_t);
    return (pstd::norm(r_parl) + pstd::norm(r_perp)) / 2;
  }

  SPECULA_CPU_GPU inline SampledSpectrum fresnel_complex(Float cos_theta_i, SampledSpectrum eta,
                                                         SampledSpectrum k) {
    SampledSpectrum result;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      result[i] = fresnel_complex(cos_theta_i, pstd::complex<Float>(eta[i], k[i]));
    }
    return result;
  }

  SPECULA_CPU_GPU Float fresnel_moment1(Float inv_eta);
  SPECULA_CPU_GPU Float fresnel_moment2(Float inv_eta);
} // namespace specula

#endif // SPECULA_UTIL_SCATTERING_FUNCTIONS_HPP
