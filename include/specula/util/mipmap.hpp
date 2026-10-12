#ifndef SPECULA_UTIL_MIPMAP_HPP
#define SPECULA_UTIL_MIPMAP_HPP

#include <cstdint>

#include <fmt/color.h>

#include "specula/types.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/image.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  enum class FilterFunction : uint8_t { POINT, BILINEAR, TRILINEAR, EWA };

  struct MipMapFilterOptions {
    FilterFunction filter = FilterFunction::EWA;
    Float max_anisotropy = 8.0f;

    bool operator<(MipMapFilterOptions o) const {
      return std::tie(filter, max_anisotropy) < std::tie(o.filter, o.max_anisotropy);
    }
  };

  class MipMap {
  public:
    static MipMap *create_from_file(const std::string &filename, const MipMapFilterOptions &options,
                                    WrapMode wrap_mode, ColorEncoding encoding, Allocator alloc);

    MipMap(Image image, const RgbColorSpace *color_space, WrapMode wrap_mode, Allocator alloc,
           const MipMapFilterOptions &options);

    [[nodiscard]] int levels() const { return int(pyramid.size()); }
    [[nodiscard]] const RgbColorSpace *get_rgb_color_space() const { return color_space; }
    [[nodiscard]] const Image &get_level(int level) const { return pyramid[level]; }
    [[nodiscard]] Point2i level_resolution(int level) const {
      ASSERT(level >= 0 && level < pyramid.size());
      return pyramid[level].resolution();
    }

    template <typename T> T filter(Point2f st, Vector2f dstdx, Vector2f dstdy) const;

  private:
    template <typename T> T texel(int level, Point2i st) const;
    template <typename T> T bilerp(int level, Point2f st) const;
    template <typename T> T ewa(int level, Point2f st, Vector2f dst0, Vector2f dst1) const;

    friend struct fmt::formatter<MipMap>;

    pstd::vector<Image> pyramid;
    const RgbColorSpace *color_space;
    WrapMode wrap_mode;
    MipMapFilterOptions options;
  };

  pstd::optional<FilterFunction> parse_filter(const char *v);
} // namespace specula

template <> struct fmt::formatter<specula::FilterFunction> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::FilterFunction &v, FormatContext &ctx) const {
    switch (v) {
    case specula::FilterFunction::POINT:
      return format_to(ctx.out(), "Point");
    case specula::FilterFunction::BILINEAR:
      return format_to(ctx.out(), "Bilinear");
    case specula::FilterFunction::TRILINEAR:
      return format_to(ctx.out(), "Trilinear");
    case specula::FilterFunction::EWA:
      return format_to(ctx.out(), "EWA");
    }
  }
};

template <> struct fmt::formatter<specula::MipMapFilterOptions> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::MipMapFilterOptions &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ MipMapFilterOptions filter={} maxAnisotropy={} ]", v.filter,
                     v.max_anisotropy);
  }
};

template <> struct fmt::formatter<specula::MipMap> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::MipMap &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ MipMap pyramid={} colorSpace={} wrapMode={} options={} ]",
                     v.pyramid, v.color_space, v.wrap_mode, v.options);
  }
};

#endif // SPECULA_UTIL_MIPMAP_HPP
