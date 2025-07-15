#ifndef INCLUDE_COLOR_COLOR_ENCODING_HPP_
#define INCLUDE_COLOR_COLOR_ENCODING_HPP_

#include "specula.hpp"
#include "specula/util/pstd/array.hpp"
#include "specula/util/pstd/span.hpp"
#include "specula/util/tagged_pointer.hpp"
#include "util/math.hpp"

namespace specula {
  class LinearColorEncoding;
  class sRgbColorEncoding;
  class GammaColorEncoding;

  extern SPECULA_CONST Float SRGB_TO_LINEAR_LUT[256];

  class ColorEncoding
      : public TaggedPointer<LinearColorEncoding, sRgbColorEncoding, GammaColorEncoding> {
  public:
    using TaggedPointer::TaggedPointer;

    SPECULA_CPU_GPU inline void to_linear(pstd::span<const uint8_t> vin,
                                          pstd::span<Float> vout) const;

    SPECULA_CPU_GPU inline void from_linear(pstd::span<const Float> vin,
                                            pstd::span<uint8_t> vout) const;

    SPECULA_CPU_GPU inline Float to_float_linear(Float v) const;

    static const ColorEncoding get(const std::string &name, Allocator allocator);
    static void initialize(Allocator allocator);

    static ColorEncoding LINEAR;
    static ColorEncoding SRGB;
  };

  class LinearColorEncoding {
  public:
    SPECULA_CPU_GPU void to_linear(pstd::span<const uint8_t> vin, pstd::span<Float> vout) const {
      DASSERT_EQ(vin.size(), vout.size());
      for (size_t i = 0; i < vin.size(); ++i) {
        vout[i] = vin[i] / 255.0f;
      }
    }

    SPECULA_CPU_GPU void from_linear(pstd::span<const Float> vin, pstd::span<uint8_t> vout) const {
      DASSERT_EQ(vin.size(), vout.size());
      for (size_t i = 0; i < vin.size(); ++i) {
        vout[i] = static_cast<uint8_t>(std::clamp(vin[i] * 255.0f + 0.5f, 0.0f, 255.0f));
      }
    }

    SPECULA_CPU_GPU Float to_float_linear(Float v) const { return v; }
  };

  class sRgbColorEncoding {
  public:
    SPECULA_CPU_GPU void to_linear(pstd::span<const uint8_t> vin, pstd::span<Float> vout) const;
    SPECULA_CPU_GPU void from_linear(pstd::span<const Float> vin, pstd::span<uint8_t> vout) const;
    SPECULA_CPU_GPU Float to_float_linear(Float v) const;
  };

  class GammaColorEncoding {
  public:
    SPECULA_CPU_GPU GammaColorEncoding(Float gamma);

    SPECULA_CPU_GPU void to_linear(pstd::span<const uint8_t> vin, pstd::span<Float> vout) const;
    SPECULA_CPU_GPU void from_linear(pstd::span<const Float> vin, pstd::span<uint8_t> vout) const;
    SPECULA_CPU_GPU Float to_float_linear(Float v) const;

  private:
    Float gamma;
    pstd::array<Float, 256> apply_lut;
    pstd::array<Float, 1024> inverse_lut;

    friend struct fmt::formatter<GammaColorEncoding>;
  };

  SPECULA_CPU_GPU inline void ColorEncoding::to_linear(pstd::span<const uint8_t> vin,
                                                       pstd::span<Float> vout) const {
    auto to_linear_impl = [&](auto ptr) { return ptr->to_linear(vin, vout); };
    dispatch(to_linear_impl);
  }

  SPECULA_CPU_GPU inline void ColorEncoding::from_linear(pstd::span<const Float> vin,
                                                         pstd::span<uint8_t> vout) const {
    auto from_linear_impl = [&](auto ptr) { return ptr->from_linear(vin, vout); };
    dispatch(from_linear_impl);
  }

  SPECULA_CPU_GPU inline Float ColorEncoding::to_float_linear(Float v) const {
    auto to_float_linear_impl = [&](auto ptr) { return ptr->to_float_linear(v); };
    return dispatch(to_float_linear_impl);
  }

  SPECULA_CPU_GPU inline Float linear_to_srgb(Float value) {
    if (value <= 0.0031308f) {
      return 12.92f * value;
    }
    float sqrt_value = safe_sqrt(value);
    float p = evaluate_polynomial(sqrt_value, -0.0016829072605308378f, 0.03453868659826638f,
                                  0.7642611304733891f, 2.0041169284241644f, 0.7551545191665577f,
                                  -0.016202083165206348f);
    float q =
        evaluate_polynomial(sqrt_value, 4.178892964897981e-7f, -0.00004375359692957097f,
                            0.03467195408529984f, 0.6085338522168684f, 1.8970238036421054f, 1.f);
    return p / q * value;
  }

  SPECULA_CPU_GPU inline uint8_t linear_to_srgb8(Float value, Float dither = 0) {
    if (value <= 0) {
      return 0;
    } else if (value >= 1) {
      return 255;
    }
    return clamp(pstd::round(255.0f * linear_to_srgb(value) + dither), 0, 255);
  }

  SPECULA_CPU_GPU inline Float srgb_to_linear(float value) {
    if (value <= 0.04045f) {
      return value * (1.0f / 12.92f);
    }
    float p = evaluate_polynomial(value, -0.0163933279112946f, -0.7386328024653209f,
                                  -11.199318357635072f, -47.46726633009393f, -36.04572663838034f);
    float q = evaluate_polynomial(value, -0.004261480793199332f, -19.140923959601675f,
                                  -59.096406619244426f, -18.225745396846637f, 1.f);
    return p / q * value;
  }

  SPECULA_CPU_GPU inline Float srgb8_to_linear(uint8_t value) { return SRGB_TO_LINEAR_LUT[value]; }
} // namespace specula

template <> struct fmt::formatter<specula::sRgbColorEncoding> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::sRgbColorEncoding &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ sRgbColorEncoding ]");
  }
};

template <> struct fmt::formatter<specula::LinearColorEncoding> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::LinearColorEncoding &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ LinearColorEncoding ]");
  }
};

template <> struct fmt::formatter<specula::GammaColorEncoding> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::GammaColorEncoding &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ GammaColorEncoding gamma={:.2f} ]", v.gamma);
  }
};

template <> struct fmt::formatter<specula::ColorEncoding> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ColorEncoding &v, FormatContext &ctx) const {
    if (!v.ptr()) {
      return format_to(ctx.out(), "(nullptr)");
    }
    auto ts = [&](auto ptr) { return format_to(ctx.out(), "{}", *ptr); };
    return v.dispatch_cpu(ts);
  }
};

#endif // INCLUDE_COLOR_COLOR_ENCODING_HPP_
