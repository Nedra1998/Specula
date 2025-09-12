#ifndef INCLUDE_IMAGE_METADATA_HPP_
#define INCLUDE_IMAGE_METADATA_HPP_

#include <map>

#include "util/colorspace.hpp"
#include "util/math/square_matrix.hpp"
#include "util/pstd/optional.hpp"
#include "util/vecmath/bounds2.hpp"
#include "util/vecmath/tuple2.hpp"

namespace specula {
  struct ImageMetadata {
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

#endif // INCLUDE_IMAGE_METADATA_HPP_
