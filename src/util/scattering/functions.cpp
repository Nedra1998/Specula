#include "specula/util/scattering/functions.hpp"

specula::Float specula::fresnel_moment1(Float inv_eta) {
  Float eta2 = inv_eta * inv_eta, eta3 = eta2 * inv_eta, eta4 = eta3 * inv_eta,
        eta5 = eta4 * inv_eta;
  if (inv_eta < 1) {
    return 0.45966f - 1.73965f * inv_eta + 3.37668f * eta2 - 3.904945f * eta3 + 2.49277f * eta4 -
           0.68441f * eta5;
  } else {
    return -4.61686f + 11.1136f * inv_eta - 10.4646f * eta2 + 5.11455f * eta3 - 1.27198f * eta4 +
           0.12746f * eta5;
  }
}

specula::Float specula::fresnel_moment2(Float inv_eta) {
  Float eta2 = inv_eta * inv_eta, eta3 = eta2 * inv_eta, eta4 = eta3 * inv_eta,
        eta5 = eta4 * inv_eta;
  if (inv_eta < 1) {
    return 0.27614f - 0.87350f * inv_eta + 1.12077f * eta2 - 0.65095f * eta3 + 0.07883f * eta4 +
           0.04860f * eta5;
  } else {
    Float r_eta = 1 / inv_eta, r_eta2 = r_eta * r_eta, r_eta3 = r_eta2 * r_eta;
    return -547.033f + 45.3087f * r_eta3 - 218.725f * r_eta2 + 458.843f * r_eta +
           404.557f * inv_eta - 189.519f * eta2 + 54.9327f * eta3 - 9.00603f * eta4 +
           0.63942f * eta5;
  }
}
