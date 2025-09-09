#ifndef INCLUDE_SPECTRUM_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_SPECTRUM_HPP_

#include "specula/specula.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/tagged_pointer.hpp"
#include "util/spectrum/constants.hpp"

namespace specula {
  class BlackbodySpectrum;
  class ConstantSpectrum;
  class PiecewiseLinearSpectrum;
  class DenslySampledSpectrum;
  class RgbAlbedoSpectrum;
  class RgbUnboundedSpectrum;
  class RgbIlluminantSpectrum;

  class Spectrum
      : public TaggedPointer<ConstantSpectrum, DenslySampledSpectrum, PiecewiseLinearSpectrum,
                             RgbAlbedoSpectrum, RgbUnboundedSpectrum, RgbIlluminantSpectrum,
                             BlackbodySpectrum> {
  public:
    using TaggedPointer::TaggedPointer;

    // TODO: These should be inlined, but that causes a number of issues with
    // ordering of class definitions in the header files.
    SPECULA_CPU_GPU Float operator()(Float lambda) const;
    SPECULA_CPU_GPU Float max_value() const;
    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const;

  private:
    std::string format_to() const;

    friend struct fmt::formatter<Spectrum>;
  };

  SPECULA_CPU_GPU inline Float inner_product(Spectrum f, Spectrum g) {
    Float integral = 0;
    for (Float lambda = LAMBDA_MIN; lambda <= LAMBDA_MAX; ++lambda) {
      integral += f(lambda) * g(lambda);
    }
    return integral;
  }

} // namespace specula

template <> struct fmt::formatter<specula::Spectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Spectrum &v, FormatContext &ctx) const {
    // The formatter must be inline, but the dispatch must be defined _after_
    // all other spectrum classes have been defined, so it is done in a private
    // function call within the spectrum to handle the dispatch.
    return format_to(ctx.out(), "{}", v.format_to());
  }
};

#endif // INCLUDE_SPECTRUM_SPECTRUM_HPP_
