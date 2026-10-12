#include "specula/util/image/image.hpp"

#include <cstddef>
#include <memory>
#include <numeric>

#include <common/TracyColor.hpp>

#include "specula/util/bluenoise.hpp"
#include "specula/util/check.hpp"
#include "specula/util/color.hpp"
#include "specula/util/image/image_channel_desc.hpp"
#include "specula/util/image/image_channel_values.hpp"
#include "specula/util/image/pixel_format.hpp"
#include "specula/util/image/wrap_mode.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  template <typename F>
  void for_extent(const Bounds2i &extent, WrapMode2D wrap_mode, const Image &image, F op) {
    ASSERT_LT(extent.p_min.x, extent.p_max.x);
    ASSERT_LT(extent.p_min.y, extent.p_max.y);

    int nx = extent.p_max[0] - extent.p_min[0];
    int nc = image.n_channels();
    if (intersect(extent, Bounds2i({0, 0}, image.resolution())) == extent) {
      for (int y = extent.p_min[1]; y < extent.p_max[1]; ++y) {
        int offset = image.pixel_offset({extent.p_min[0], y});
        for (int x = 0; x < nx; ++x) {
          for (int c = 0; c < nc; ++c) {
            op(offset++);
          }
        }
      }
    } else {
      for (int y = extent.p_min[1]; y < extent.p_max[1]; ++y) {
        for (int x = 0; x < nx; ++x) {
          Point2i p(extent.p_min[0] + x, y);
          // TODO: This returns false for black wrap mode
          ASSERT(remap_pixel_coords(&p, image.resolution(), wrap_mode));
          int offset = image.pixel_offset(p);
          for (int c = 0; c < nc; ++c) {
            op(offset++);
          }
        }
      }
    }
  }
} // namespace specula

specula::pstd::vector<specula::Image>
specula::Image::generate_pyramid(Image image, WrapMode2D wrap_mode, Allocator alloc) {
  PixelFormat orig_format = image.format_;
  int n_channels = image.n_channels();
  ColorEncoding orig_encoding = image.encoding_;

  if (!is_power_of_2(image.resolution_.x) || !is_power_of_2(image.resolution_.y)) {
    image = image.float_resize_up(
        Point2i(round_up_pow2(image.resolution_.x), round_up_pow2(image.resolution_.y)), wrap_mode);
  } else if (!is_32bit(image.format_)) {
    image = image.convert_to_format(PixelFormat::FLOAT);
  }
  ASSERT(is_32bit(image.format_));

  int nlevels = 1 + log2_int(std::max(image.resolution_.x, image.resolution_.y));
  pstd::vector<Image> pyramid(alloc);
  pyramid.reserve(nlevels);

  for (int i = 0; i < nlevels - 1; ++i) {
    pyramid.push_back(
        Image(orig_format, image.resolution_, image.channel_names_, orig_encoding, alloc));
    Point2i next_resolution(std::max(1, image.resolution_.x / 2),
                            std::max(1, image.resolution_.y / 2));
    Image next_image(image.format_, next_resolution, image.channel_names_, orig_encoding);

    int src_deltas[4] = {0, n_channels, n_channels * image.resolution_.x,
                         n_channels * (image.resolution_.x + 1)};
    if (image.resolution_.x == 1) {
      src_deltas[1] = 0;
      src_deltas[3] -= n_channels;
    }
    if (image.resolution_.y == 1) {
      src_deltas[2] = 0;
      src_deltas[3] -= n_channels * image.resolution_.x;
    }

    parallel_for(0, next_resolution.y, [&](int64_t y) {
      int src_offset = image.pixel_offset(Point2i(0, 2 * y));
      int next_offset = next_image.pixel_offset(Point2i(0, y));

      for (int x = 0; x < next_resolution.x; ++x, src_offset += n_channels) {
        for (int c = 0; c < n_channels; ++c, ++src_offset, ++next_offset) {
          next_image.p32[next_offset] =
              (image.p32[src_offset] + image.p32[src_offset + src_deltas[1]] +
               image.p32[src_offset + src_deltas[2]] + image.p32[src_offset + src_deltas[3]]) /
              4;
        }
      }

      int ystart = 2 * y;
      int yend = std::min(2 * int(y) + 2, image.resolution_.y);
      int offset = image.pixel_offset({0, ystart});
      int count = (yend - ystart) * n_channels * image.resolution_.x;
      pyramid[i].copy_rect_in(Bounds2i({0, ystart}, {image.resolution_.x, yend}),
                              pstd::span<const float>(image.p32.data() + offset, count));
    });

    image = std::move(next_image);
  }

  ASSERT(image.resolution_.x == 1 && image.resolution_.y == 1);
  pyramid.push_back(Image(orig_format, {1, 1}, image.channel_names_, orig_encoding, alloc));
  pyramid[nlevels - 1].copy_rect_in(Bounds2i({0, 0}, {1, 1}),
                                    pstd::span<const float>(image.p32.data(), n_channels));

  return pyramid;
}

specula::Image::Image(pstd::vector<uint8_t> p8, Point2i resolution,
                      pstd::span<const std::string> channels, ColorEncoding encoding)
    : format_(PixelFormat::U256), resolution_(resolution),
      channel_names_(channels.begin(), channels.end()), encoding_(encoding), p8(std::move(p8)) {
  ASSERT_EQ(p8.size(), n_channels() * resolution_.x * resolution_.y);
}

specula::Image::Image(pstd::vector<Half> p16, Point2i resolution,
                      pstd::span<const std::string> channels)
    : format_(PixelFormat::HALF), resolution_(resolution),
      channel_names_(channels.begin(), channels.end()), p16(std::move(p16)) {
  ASSERT_EQ(p16.size(), n_channels() * resolution_.x * resolution_.y);
}

specula::Image::Image(pstd::vector<float> p32, Point2i resolution,
                      pstd::span<const std::string> channels)
    : format_(PixelFormat::FLOAT), resolution_(resolution),
      channel_names_(channels.begin(), channels.end()), p32(std::move(p32)) {
  ASSERT_EQ(p32.size(), n_channels() * resolution_.x * resolution_.y);
}

specula::Image::Image(PixelFormat format, Point2i resolution,
                      pstd::span<const std::string> channels, ColorEncoding encoding,
                      Allocator alloc)
    : format_(format), resolution_(resolution), channel_names_(channels.begin(), channels.end()),
      encoding_(encoding), p8(alloc), p16(alloc), p32(alloc) {
  if (is_8bit(format_)) {
    p8.resize(n_channels() * size_t(resolution_.x) * size_t(resolution_.y));
    ASSERT(encoding_);
  } else if (is_16bit(format_)) {
    p16.resize(n_channels() * size_t(resolution_.x) * size_t(resolution_.y));
  } else {
    ASSERT(is_32bit(format_));
    p32.resize(n_channels() * size_t(resolution_.x) * size_t(resolution_.y));
  }
}

specula::ImageChannelValues specula::Image::get_channels(Point2i p, WrapMode2D wrap_mode) const {
  ImageChannelValues cv(n_channels(), Float(0));
  if (!remap_pixel_coords(&p, resolution_, wrap_mode)) {
    return cv;
  }

  size_t poffset = pixel_offset(p);
  switch (format_) {
  case PixelFormat::U256:
    encoding_.to_linear({&p8[poffset], n_channels()}, {cv.data(), n_channels()});
    break;
  case PixelFormat::HALF:
    for (int i = 0; i < n_channels(); ++i) {
      cv[i] = Float(p16[poffset + i]);
    }
    break;
  case PixelFormat::FLOAT:
    for (int i = 0; i < n_channels(); ++i) {
      cv[i] = p32[poffset + i];
    }
    break;
  }

  return cv;
}

specula::ImageChannelValues specula::Image::get_channels(Point2i p, const ImageChannelDesc &desc,
                                                         WrapMode2D wrap_mode) const {
  ImageChannelValues cv(desc.offset.size(), Float(0));
  if (!remap_pixel_coords(&p, resolution_, wrap_mode)) {
    return cv;
  }

  size_t poffset = pixel_offset(p);
  switch (format_) {
  case PixelFormat::U256:
    for (int i = 0; i < desc.offset.size(); ++i) {
      encoding_.to_linear({&p8[poffset + desc.offset[i]], 1}, {&cv[i], 1});
    }
    break;
  case PixelFormat::HALF:
    for (int i = 0; i < desc.offset.size(); ++i) {
      cv[i] = Float(p16[poffset + desc.offset[i]]);
    }
    break;
  case PixelFormat::FLOAT:
    for (int i = 0; i < desc.offset.size(); ++i) {
      cv[i] = p32[poffset + desc.offset[i]];
    }
    break;
  }

  return cv;
}

specula::ImageChannelDesc
specula::Image::get_channel_desc(pstd::span<const std::string> channels) const {
  ImageChannelDesc desc;
  desc.offset.resize(channels.size());
  for (size_t i = 0; i < channels.size(); ++i) {
    size_t j = 0;
    for (j = 0; j < channel_names_.size(); ++j) {
      if (channels[i] == channel_names_[j]) {
        desc.offset[i] = j;
        break;
      }
    }
    if (j == channel_names_.size()) {
      return {};
    }
  }
  return desc;
}

void specula::Image::set_channels(Point2i p, const ImageChannelValues &values) {
  ASSERT_LE(values.size(), n_channels());
  int i = 0;
  for (const auto *iter = values.begin(); iter != values.end(); ++iter, ++i) {
    set_channel(p, i, *iter);
  }
}

void specula::Image::set_channels(Point2i p, pstd::span<const Float> values) {
  ASSERT_LE(values.size(), n_channels());
  for (size_t i = 0; i < values.size(); ++i) {
    set_channel(p, i, values[i]);
  }
}

void specula::Image::set_channels(Point2i p, const ImageChannelDesc &desc,
                                  pstd::span<const Float> values) {
  ASSERT_LE(values.size(), n_channels());
  for (size_t i = 0; i < values.size(); ++i) {
    set_channel(p, desc.offset[i], values[i]);
  }
}

specula::ImageChannelValues specula::Image::lookup_nearest(Point2f p, WrapMode2D wrap_mode) const {
  ImageChannelValues cv(n_channels(), Float(0));
  for (int c = 0; c < n_channels(); ++c) {
    cv[c] = lookup_nearest_channel(p, c, wrap_mode);
  }
  return cv;
}

specula::ImageChannelValues specula::Image::lookup_nearest(Point2f p, const ImageChannelDesc &desc,
                                                           WrapMode2D wrap_mode) const {
  ImageChannelValues cv(desc.offset.size(), Float(0));
  for (int i = 0; i < desc.offset.size(); ++i) {
    cv[i] = lookup_nearest_channel(p, desc.offset[i], wrap_mode);
  }
  return cv;
}

specula::ImageChannelValues specula::Image::bilerp(Point2f p, WrapMode2D wrap_mode) const {
  ImageChannelValues cv(n_channels(), Float(0));
  for (int c = 0; c < n_channels(); ++c) {
    cv[c] = bilerp_channel(p, c, wrap_mode);
  }
  return cv;
}

specula::ImageChannelValues specula::Image::bilerp(Point2f p, const ImageChannelDesc &desc,
                                                   WrapMode2D wrap_mode) const {
  ImageChannelValues cv(desc.offset.size(), Float(0));
  for (int i = 0; i < desc.offset.size(); ++i) {
    cv[i] = bilerp_channel(p, desc.offset[i], wrap_mode);
  }
  return cv;
}

specula::Image specula::Image::select_channels(const ImageChannelDesc &desc,
                                               Allocator alloc) const {
  std::vector<std::string> desc_channel_names;
  desc_channel_names.reserve(desc.offset.size());
  for (int i : desc.offset) {
    desc_channel_names.push_back(channel_names_[i]);
  }

  Image image(format_, resolution_, desc_channel_names, encoding_, alloc);
  switch (format_) {
  case PixelFormat::U256:
    for (int y = 0; y < resolution_.y; ++y) {
      for (int x = 0; x < resolution_.x; ++x) {
        const auto *src = (const uint8_t *)raw_pointer({x, y});
        auto *dst = (uint8_t *)image.raw_pointer({x, y});
        for (size_t i = 0; i < desc.offset.size(); ++i) {
          dst[i] = src[desc.offset[i]];
        }
      }
    }
    break;
  case PixelFormat::HALF:
    for (int y = 0; y < resolution_.y; ++y) {
      for (int x = 0; x < resolution_.x; ++x) {
        const auto *src = (const Half *)raw_pointer({x, y});
        auto *dst = (Half *)image.raw_pointer({x, y});
        for (size_t i = 0; i < desc.offset.size(); ++i) {
          dst[i] = src[desc.offset[i]];
        }
      }
    }
    break;
  case PixelFormat::FLOAT:
    for (int y = 0; y < resolution_.y; ++y) {
      for (int x = 0; x < resolution_.x; ++x) {
        const auto *src = (const float *)raw_pointer({x, y});
        auto *dst = (float *)image.raw_pointer({x, y});
        for (size_t i = 0; i < desc.offset.size(); ++i) {
          dst[i] = src[desc.offset[i]];
        }
      }
    }
    break;
  }

  return image;
}

specula::Image specula::Image::crop(const Bounds2i &bounds, Allocator alloc) const {
  ASSERT_GT(bounds.area(), 0);
  ASSERT(bounds.p_min.x >= 0 && bounds.p_min.y >= 0);
  Image image(format_, Point2i(bounds.p_max - bounds.p_min), channel_names_, encoding_, alloc);

  for (Point2i p : bounds) {
    for (int c = 0; c < n_channels(); ++c) {
      image.set_channel(Point2i(p - bounds.p_min), c, get_channel(p, c));
    }
  }

  return image;
}

void specula::Image::copy_rect_out(const Bounds2i &extent, pstd::span<float> buf,
                                   WrapMode2D wrap_mode) const {
  ASSERT_GE(buf.size(), extent.area() * n_channels());

  auto *iter = buf.begin();
  switch (format_) {
  case PixelFormat::U256:
    if (intersect(extent, Bounds2i({0, 0}, resolution_)) == extent) {
      size_t count = n_channels() * (extent.p_max.x - extent.p_min.x);
      for (int y = extent.p_min.y; y < extent.p_max.y; ++y) {
        size_t offset = pixel_offset({extent.p_min.x, y});
#ifdef SPECULA_FLOAT_AS_DOUBLE
        for (int i = 0; i < count; ++i) {
          Float v;
          encoding_.to_linear({&p8[offset + i], 1}, {&v, 1});
          *iter++ = v;
        }
#else
        encoding_.to_linear({&p8[offset], count}, {&*iter, count});
        iter += count;
#endif
      }
    } else {
      for_extent(extent, wrap_mode, *this, [&iter, this](int offset) {
#ifdef SPECULA_FLOAT_AS_DOUBLE
        Float v;
        encoding.to_linear({&p8[offset], 1}, {&v, 1});
        *iter = v;
#else
          encoding_.to_linear({&p8[offset], 1}, {&*iter, 1});
#endif
        ++iter;
      });
    }
    break;
  case PixelFormat::HALF:
    for_extent(extent, wrap_mode, *this,
               [&iter, this](int offset) { *iter++ = Float(p16[offset]); });
  case PixelFormat::FLOAT:
    for_extent(extent, wrap_mode, *this,
               [&iter, this](int offset) { *iter++ = Float(p32[offset]); });
    break;
  }
}

void specula::Image::copy_rect_in(const Bounds2i &extent, pstd::span<const float> buf) {
  ASSERT_GE(buf.size(), extent.area() * n_channels());

  const auto *iter = buf.begin();
  switch (format_) {
  case PixelFormat::U256:
    if (intersect(extent, Bounds2i({0, 0}, resolution_)) == extent) {
      size_t count = n_channels() * (extent.p_max.x - extent.p_min.x);
      for (int y = extent.p_min.y; y < extent.p_max.y; ++y) {
        size_t offset = pixel_offset({extent.p_min.x, y});
#ifdef SPECULA_FLOAT_AS_DOUBLE
        for (int i = 0; i < count; ++i) {
          Float v = *iter++;
          encoding_.from_linear({&v, 1}, {&p8[offset + i], 1});
        }
#else
        encoding_.from_linear({&*iter, count}, {&p8[offset], count});
        iter += count;
#endif
      }
    } else {
      for_extent(extent, WrapMode::CLAMP, *this, [&iter, this](int offset) {
        Float v = *iter++;
        encoding_.from_linear({&v, 1}, {&p8[offset], 1});
      });
    }
    break;
  case PixelFormat::HALF:
    for_extent(extent, WrapMode::CLAMP, *this,
               [&iter, this](int offset) { p16[offset] = Half(*iter++); });
    break;
  case PixelFormat::FLOAT:
    for_extent(extent, WrapMode::CLAMP, *this,
               [&iter, this](int offset) { p32[offset] = *iter++; });
    break;
  }
}

specula::ImageChannelValues specula::Image::average(const ImageChannelDesc &desc) const {
  std::vector<double> sum(desc.size(), 0.0);

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      ImageChannelValues v = get_channels({x, y}, desc);
      for (int c = 0; c < desc.size(); ++c) {
        sum[c] += v[c];
      }
    }
  }

  ImageChannelValues average(desc.size());
  for (int c = 0; c < desc.size(); ++c) {
    average[c] = sum[c] / (Float(resolution_.x * resolution_.y));
  }
  return average;
}

bool specula::Image::has_any_infinite_pixels() const {
  if (format_ == PixelFormat::U256) {
    return false;
  }

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      for (int c = 0; c < n_channels(); ++c) {
        if (isinf(get_channel({x, y}, c))) {
          return true;
        }
      }
    }
  }

  return false;
}

bool specula::Image::has_any_nan_pixels() const {
  if (format_ == PixelFormat::U256) {
    return false;
  }

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      for (int c = 0; c < n_channels(); ++c) {
        if (isnan(get_channel({x, y}, c))) {
          return true;
        }
      }
    }
  }

  return false;
}

specula::ImageChannelValues specula::Image::mae(const ImageChannelDesc &desc, const Image &ref,
                                                Image *error_image) const {
  std::vector<double> sum_error(desc.size(), 0.0);

  ImageChannelDesc ref_desc = ref.get_channel_desc(channel_names(desc));
  ASSERT((bool)ref_desc);
  ASSERT_EQ(resolution(), ref.resolution());

  if (error_image != nullptr) {
    *error_image = Image(PixelFormat::FLOAT, resolution_, channel_names_);
  }

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      ImageChannelValues v = get_channels({x, y}, desc);
      ImageChannelValues vref = ref.get_channels({x, y}, ref_desc);

      for (int c = 0; c < desc.size(); ++c) {
        double error = std::abs(double(v[c]) - double(vref[c]));
        if (isinf(error)) {
          continue;
        }
        sum_error[c] += error;
        if (error_image != nullptr) {
          error_image->set_channel({x, y}, c, error);
        }
      }
    }
  }

  ImageChannelValues error(desc.size());
  for (int c = 0; c < desc.size(); ++c) {
    error[c] = sum_error[c] / (Float(resolution_.x * resolution_.y));
  }
  return error;
}

specula::ImageChannelValues specula::Image::mse(const ImageChannelDesc &desc, const Image &ref,
                                                Image *error_image) const {
  std::vector<double> sum_error(desc.size(), 0.0);
  ImageChannelDesc ref_desc = ref.get_channel_desc(channel_names(desc));
  ASSERT((bool)ref_desc);
  ASSERT_EQ(resolution(), ref.resolution());

  if (error_image != nullptr) {
    *error_image = Image(PixelFormat::FLOAT, resolution_, channel_names_);
  }

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      ImageChannelValues v = get_channels({x, y}, desc);
      ImageChannelValues vref = ref.get_channels({x, y}, ref_desc);

      for (int c = 0; c < desc.size(); ++c) {
        double error = sqr(double(v[c]) - double(vref[c]));
        if (isinf(error)) {
          continue;
        }
        sum_error[c] += error;
        if (error_image != nullptr) {
          error_image->set_channel({x, y}, c, error);
        }
      }
    }
  }

  ImageChannelValues error(desc.size());
  for (int c = 0; c < desc.size(); ++c) {
    error[c] = sum_error[c] / (Float(resolution_.x * resolution_.y));
  }
  return error;
}

specula::ImageChannelValues specula::Image::mrse(const ImageChannelDesc &desc, const Image &ref,
                                                 Image *error_image) const {
  std::vector<double> sum_error(desc.size(), 0.0);
  ImageChannelDesc ref_desc = ref.get_channel_desc(channel_names(desc));
  ASSERT((bool)ref_desc);
  ASSERT_EQ(resolution(), ref.resolution());

  if (error_image != nullptr) {
    *error_image = Image(PixelFormat::FLOAT, resolution_, channel_names_);
  }

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      ImageChannelValues v = get_channels({x, y}, desc);
      ImageChannelValues vref = ref.get_channels({x, y}, ref_desc);

      for (int c = 0; c < desc.size(); ++c) {
        double error = sqr(double(v[c]) - double(vref[c])) / sqr(vref[c] + 0.01);
        if (isinf(error)) {
          continue;
        }
        sum_error[c] += error;
        if (error_image != nullptr) {
          error_image->set_channel({x, y}, c, error);
        }
      }
    }
  }

  ImageChannelValues error(desc.size());
  for (int c = 0; c < desc.size(); ++c) {
    error[c] = sum_error[c] / (Float(resolution_.x * resolution_.y));
  }
  return error;
}

specula::Image specula::Image::gaussian_filter(const ImageChannelDesc &desc, int half_width,
                                               Float sigma) const {
  std::vector<Float> wts(2 * half_width + 1, Float(0));
  for (int d = 0; d < 2 * half_width + 1; ++d) {
    wts[d] = gaussian(d - half_width, 0, sigma);
  }

  Float wt_sum = std::accumulate(wts.begin(), wts.end(), Float(0));
  for (Float &w : wts) {
    w /= wt_sum;
  }

  Image blurx(PixelFormat::FLOAT, resolution_, channel_names(desc));
  int nc = desc.size();
  parallel_for(0, resolution_.y, [&](int64_t y0, int64_t y1) {
    for (int y = y0; y < y1; ++y) {
      for (int x = 0; x < resolution_.x; ++x) {
        ImageChannelValues result(desc.size());
        for (int r = -half_width; r <= half_width; ++r) {
          ImageChannelValues cv = get_channels({x + r, y}, desc);
          for (int c = 0; c < nc; ++c) {
            result[c] += wts[r + half_width] * cv[c];
          }
        }
        blurx.set_channels({x, y}, result);
      }
    }
  });

  Image blury(PixelFormat::FLOAT, resolution_, channel_names(desc));
  parallel_for(0, resolution_.y, [&](int64_t y0, int64_t y1) {
    for (int y = y0; y < y1; ++y) {
      for (int x = 0; x < resolution_.x; ++x) {
        ImageChannelValues result(desc.size());
        for (int r = -half_width; r <= half_width; ++r) {
          ImageChannelValues cv = blurx.get_channels({x, y + r});
          for (int c = 0; c < nc; ++c) {
            result[c] += wts[r + half_width] * cv[c];
          }
        }
        blury.set_channels({x, y}, result);
      }
    }
  });

  return blury;
}

specula::Image specula::Image::joint_bilateral_filter(const ImageChannelDesc &to_filter,
                                                      int half_width, const Float xy_sigma[2],
                                                      const ImageChannelDesc &joint,
                                                      const ImageChannelValues &joint_sigma) const {
  ASSERT_EQ(joint.size(), joint_sigma.size());
  Image result(PixelFormat::FLOAT, resolution_, channel_names(to_filter));

  std::vector<Float> fx, fy;
  for (int i = 0; i <= half_width; ++i) {
    fx.push_back(gaussian(i, 0, xy_sigma[0]));
    fy.push_back(gaussian(i, 0, xy_sigma[1]));
  }

  parallel_for(0, resolution_.y, [&](int64_t y0, int64_t y1) {
    for (int y = y0; y < y1; ++y) {
      for (int x = 0; x < resolution_.x; ++x) {
        ImageChannelValues joint_pixel_channels = get_channels({x, y}, joint);
        ImageChannelValues filter_sum(to_filter.size(), Float(0));
        Float weight_sum = 0;

        for (int dy = -half_width + 1; dy < half_width; ++dy) {
          if (y + dy < 0 || y + dy >= resolution_.y) {
            continue;
          }
          for (int dx = -half_width + 1; dx < half_width; ++dx) {
            if (x + dx < 0 || x + dx >= resolution_.x) {
              continue;
            }

            ImageChannelValues joint_other_channels = get_channels({x + dx, y + dy}, joint);
            Float weight = fx[std::abs(dx)] * fy[std::abs(dy)];
            for (int c = 0; c < joint.size(); ++c) {
              weight *= gaussian(joint_pixel_channels[c], joint_other_channels[c], joint_sigma[c]);
            }
            weight_sum += weight;

            ImageChannelValues filter_channels = get_channels({x + dx, y + dy}, to_filter);
            for (int c = 0; c < filter_channels.size(); ++c) {
              filter_sum[c] += weight * filter_channels[c];
            }
          }
        }

        if (weight_sum > 0) {
          for (float &c : filter_sum) {
            c /= weight_sum;
          }
        }
        result.set_channels({x, y}, filter_sum);
      }
    }
  });

  return result;
}

specula::Image specula::Image::convert_to_format(PixelFormat format, ColorEncoding encoding) const {
  if (format == format_) {
    return *this;
  }

  Image new_image(format, resolution_, channel_names_, encoding);
  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      for (int c = 0; c < n_channels(); ++c) {
        new_image.set_channel({x, y}, c, get_channel({x, y}, c));
      }
    }
  }
  return new_image;
}

specula::Image specula::Image::float_resize_up(Point2i new_resolution, WrapMode2D wrap_mode) const {
  ASSERT_GE(new_resolution.x, resolution_.x);
  ASSERT_GE(new_resolution.y, resolution_.y);

  Image resampled_image(PixelFormat::FLOAT, new_resolution, channel_names_);
  std::vector<ResampleWeight> xweights, yweights;
  xweights = resample_weights(resolution_[0], new_resolution[0]);
  yweights = resample_weights(resolution_[1], new_resolution[1]);

  parallel_for_2d(Bounds2i({0, 0}, new_resolution), [&](Bounds2i out_extent) {
    Bounds2i in_extent(
        Point2i(xweights[out_extent.p_min.x].first_pixel, yweights[out_extent.p_min.y].first_pixel),
        Point2i(xweights[out_extent.p_max.x - 1].first_pixel + 4,
                yweights[out_extent.p_max.y - 1].first_pixel + 4));
    std::vector<float> in_buf(n_channels() * in_extent.area());
    copy_rect_out(in_extent, pstd::span<float>(in_buf), wrap_mode);

    int nx_out = out_extent.p_max.x - out_extent.p_min.x;
    int ny_out = out_extent.p_max.y - out_extent.p_min.y;
    int nx_in = in_extent.p_max.x - in_extent.p_min.x;
    int ny_in = in_extent.p_max.y - in_extent.p_min.y;
    std::vector<float> xbuf(n_channels() * ny_in * nx_out);

    int xbuf_offset = 0;
    for (int yout = in_extent.p_min.y; yout < in_extent.p_max.y; ++yout) {
      for (int xout = out_extent.p_min.x; xout < out_extent.p_max.x; ++xout) {
        DASSERT(xout >= 0 && xout < xweights.size());
        const ResampleWeight &rsw = xweights[xout];

        int xin = rsw.first_pixel - in_extent.p_min.x;
        DASSERT_GE(xin, 0);
        DASSERT_LT(xin + 3, nx_in);
        int yin = yout - in_extent.p_min.y;
        int in_offset = n_channels() * (xin + yin * nx_in);
        DASSERT_GE(in_offset, 0);
        DASSERT_LT(in_offset + 3 * n_channels(), in_buf.size());

        for (int c = 0; c < n_channels(); ++c, ++xbuf_offset, ++in_offset) {
          xbuf[xbuf_offset] = rsw.weight[0] * in_buf[in_offset] +
                              rsw.weight[1] * in_buf[in_offset + n_channels()] +
                              rsw.weight[2] * in_buf[in_offset + 2 * n_channels()] +
                              rsw.weight[3] * in_buf[in_offset + 3 * n_channels()];
        }
      }
    }

    std::vector<float> out_buf(n_channels() * nx_out * ny_out);
    for (int x = 0; x < nx_out; ++x) {
      for (int y = 0; y < ny_out; ++y) {
        int yout = y + out_extent[0][1];
        DASSERT(yout >= 0 && yout < yweights.size());
        const ResampleWeight &rsw = yweights[yout];

        DASSERT_GE(rsw.first_pixel - in_extent[0][1], 0);
        int xbuf_offset = n_channels() * (x + nx_out * (rsw.first_pixel - in_extent[0][1]));
        DASSERT_GE(xbuf_offset, 0);
        int step = n_channels() * nx_out;
        DASSERT_LT(xbuf_offset + 3 * step, xbuf.size());

        int out_offset = n_channels() * (x + y * nx_out);

        for (int c = 0; c < n_channels(); ++c, ++out_offset, ++xbuf_offset) {
          out_buf[out_offset] = rsw.weight[0] * xbuf[xbuf_offset] +
                                rsw.weight[1] * xbuf[xbuf_offset + step] +
                                rsw.weight[2] * xbuf[xbuf_offset + 2 * step] +
                                rsw.weight[3] * xbuf[xbuf_offset + 3 * step];
        }
      }
    }

    resampled_image.copy_rect_in(out_extent, out_buf);
  });

  return resampled_image;
}

void specula::Image::flip_y() {
  for (int y = 0; y < resolution_.y / 2; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      size_t o1 = pixel_offset({x, y}), o2 = pixel_offset({x, resolution_.y - 1 - y});
      for (int c = 0; c < n_channels(); ++c) {
        if (is_8bit(format_)) {
          pstd::swap(p8[o1 + c], p8[o2 + c]);
        } else if (is_16bit(format_)) {
          pstd::swap(p16[o1 + c], p16[o2 + c]);
        } else {
          ASSERT(is_32bit(format_));
          pstd::swap(p32[o1 + c], p32[o2 + c]);
        }
      }
    }
  }
}

std::vector<specula::ResampleWeight> specula::Image::resample_weights(int old_res, int new_res) {
  ASSERT_GE(new_res, old_res);
  std::vector<ResampleWeight> wt(new_res);
  Float filter_radius = 2, tau = 2;

  for (int i = 0; i < new_res; ++i) {
    Float center = (i + 0.5f) * old_res / new_res;
    wt[i].first_pixel = pstd::floor((center - filter_radius) + 0.5f);
    for (int j = 0; j < 4; ++j) {
      Float pos = wt[i].first_pixel + j + 0.5f;
      wt[i].weight[j] = windowed_sinc(pos - center, filter_radius, tau);
    }

    Float inv_sum_wts = 1 / (wt[i].weight[0] + wt[i].weight[1] + wt[i].weight[2] + wt[i].weight[3]);
    for (float &j : wt[i].weight) {
      j *= inv_sum_wts;
    }
  }

  return wt;
}

std::unique_ptr<uint8_t[]> specula::Image::quantize_pixels_to_u256(int *n_out_of_gamut) const {
  std::unique_ptr<uint8_t[]> u256 =
      std::make_unique<uint8_t[]>(n_channels() * size_t(resolution_.x) * size_t(resolution_.y));

  for (int y = 0; y < resolution_.y; ++y) {
    for (int x = 0; x < resolution_.x; ++x) {
      for (int c = 0; c < n_channels(); ++c) {
        Float dither = -0.5f + blue_noise(c, {x, y});
        Float v = get_channel({x, y}, c);
        if (v < 0 || v > 1) {
          ++(*n_out_of_gamut);
        }
        u256[n_channels() * (y * resolution_.x + x) + c] = linear_to_srgb8(v, dither);
      }
    }
  }

  return u256;
}
