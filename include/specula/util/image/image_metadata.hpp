#ifndef SPECULA_UTIL_IMAGE_IMAGE_METADATA_HPP
#define SPECULA_UTIL_IMAGE_IMAGE_METADATA_HPP

#include <map>
#include <string>
#include <vector>

#include "specula/util/colorspace.hpp"
#include "specula/util/pstd.hpp"

namespace specula {
  struct ImageMetadata {
    [[nodiscard]] inline const RgbColorSpace *get_color_space() const {
      if (color_space && ((*color_space) != nullptr)) {
        return *color_space;
      }
      return RgbColorSpace::SRGB;
    }

    pstd::optional<float> render_time_seconds;
    pstd::optional<SquareMatrix<4>> camera_from_world, ndc_from_world;
    pstd::optional<Bounds2i> pixel_bounds;
    pstd::optional<Point2i> full_resolution;
    pstd::optional<int> samples_per_pixel;
    pstd::optional<float> mse;
    pstd::optional<const RgbColorSpace *> color_space;
    std::map<std::string, std::string> strings;
    std::map<std::string, std::vector<std::string>> string_vectors;
  };
} // namespace specula

template <> struct fmt::formatter<specula::ImageMetadata> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ImageMetadata &v, FormatContext &ctx) const {
    return format_to(ctx.out(),
                     "[ ImageMetadata renderTimeSeconds={} cameraFromWorld={} NDCFromWorld={} "
                     "pixelBounds={} fullResolution={} samplesPerPixel={} MSD={} colorSpace={} ]",
                     v.render_time_seconds, v.camera_from_world, v.ndc_from_world, v.pixel_bounds,
                     v.full_resolution, v.samples_per_pixel, v.mse, v.color_space);
  }
};

#endif // SPECULA_UTIL_IMAGE_IMAGE_METADATA_HPP
