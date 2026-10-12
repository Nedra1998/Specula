#include "specula/util/mipmap.hpp"

#include <algorithm>
#include <cstdlib>

#include <fmt/color.h>
#include <magic_enum/magic_enum.hpp>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/image/image.hpp"
#include "specula/util/log.hpp"
#include "specula/util/stats.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  STAT_MEMORY_COUNTER("Memory/Image maps", IMAGE_MAP_BYTES);
}

static constexpr int MIP_FILTER_LUT_SIZE = 128;
static SPECULA_CONST specula::Float MIP_FILTER_LUT[MIP_FILTER_LUT_SIZE] = {
    0.864664733f,  0.849040031f,   0.83365953f,   0.818519294f,
    0.80361563f,   0.788944781f,   0.774503231f,  0.760287285f,
    0.746293485f,  0.732518315f,   0.718958378f,  0.705610275f,
    0.692470789f,  0.679536581f,   0.666804492f,  0.654271305f,
    0.641933978f,  0.629789352f,   0.617834508f,  0.606066525f,
    0.594482362f,  0.583079159f,   0.571854174f,  0.560804546f,
    0.549927592f,  0.539220572f,   0.528680861f,  0.518305838f,
    0.50809288f,   0.498039544f,   0.488143265f,  0.478401601f,
    0.468812168f,  0.45937258f,    0.450080454f,  0.440933526f,
    0.431929469f,  0.423066139f,   0.414341331f,  0.405752778f,
    0.397298455f,  0.388976216f,   0.380784035f,  0.372719884f,
    0.364781618f,  0.356967449f,   0.34927541f,   0.341703475f,
    0.334249914f,  0.32691282f,    0.319690347f,  0.312580705f,
    0.305582166f,  0.298692942f,   0.291911423f,  0.285235822f,
    0.278664529f,  0.272195935f,   0.265828371f,  0.259560347f,
    0.253390193f,  0.247316495f,   0.241337672f,  0.235452279f,
    0.229658857f,  0.223955944f,   0.21834214f,   0.212816045f,
    0.207376286f,  0.202021524f,   0.196750447f,  0.191561714f,
    0.186454013f,  0.181426153f,   0.176476851f,  0.171604887f,
    0.166809067f,  0.162088141f,   0.157441005f,  0.152866468f,
    0.148363426f,  0.143930718f,   0.139567271f,  0.135272011f,
    0.131043866f,  0.126881793f,   0.122784719f,  0.11875169f,
    0.114781633f,  0.11087364f,    0.107026696f,  0.103239879f,
    0.0995122194f, 0.0958427936f,  0.0922307223f, 0.0886750817f,
    0.0851749927f, 0.0817295909f,  0.0783380121f, 0.0749994367f,
    0.0717130303f, 0.0684779733f,  0.0652934611f, 0.0621587038f,
    0.0590728968f, 0.0560353249f,  0.0530452281f, 0.0501018465f,
    0.0472044498f, 0.0443523228f,  0.0415447652f, 0.0387810767f,
    0.0360605568f, 0.0333825648f,  0.0307464004f, 0.0281514227f,
    0.0255970061f, 0.0230824798f,  0.0206072628f, 0.0181707144f,
    0.0157722086f, 0.013411209f,   0.0110870898f, 0.0087992847f,
    0.0065472275f, 0.00433036685f, 0.0021481365f, 0.f};

specula::MipMap *specula::MipMap::create_from_file(const std::string &filename,
                                                   const MipMapFilterOptions &options,
                                                   WrapMode wrap_mode, ColorEncoding encoding,
                                                   Allocator alloc) {
  ImageAndMetadata image_and_metadata = Image::read(filename, alloc, encoding);
  Image &image = image_and_metadata.image;
  if (image.n_channels() != 1) {
    ImageChannelDesc rgba_desc = image.get_channel_desc({"R", "G", "B", "A"});
    ImageChannelDesc rgb_desc = image.get_channel_desc({"R", "G", "B"});
    if (rgba_desc) {
      bool all_one = true;
      for (int y = 0; y < image.resolution().y; ++y) {
        for (int x = 0; x < image.resolution().x; ++x) {
          if (image.get_channels({x, y}, rgba_desc)[3] != 1) {
            all_one = false;
          }
        }
      }
      if (all_one) {
        image = image.select_channels(rgb_desc, alloc);
      } else {
        image = image.select_channels(rgba_desc, alloc);
      }
    } else {
      if (rgb_desc) {
        image = image.select_channels(rgb_desc, alloc);
      } else {
        LOG_CRITICAL("{}: image doesn't have R, G, and B channels", filename);
        std::quick_exit(1);
      }
    }
  }

  const RgbColorSpace *color_space = image_and_metadata.metadata.get_color_space();
  return alloc.new_object<MipMap>(std::move(image), color_space, wrap_mode, alloc, options);
}

specula::MipMap::MipMap(Image image, const RgbColorSpace *color_space, WrapMode wrap_mode,
                        Allocator alloc, const MipMapFilterOptions &options)
    : color_space(color_space), wrap_mode(wrap_mode), options(options) {
  ASSERT(color_space);
  pyramid = Image::generate_pyramid(std::move(image), wrap_mode, alloc);
  // TODO: Add global options control
  std::ranges::for_each(pyramid, [](const Image &im) { IMAGE_MAP_BYTES += im.bytes_used(); });
}

template <typename T> T specula::MipMap::filter(Point2f st, Vector2f dstdx, Vector2f dstdy) const {
  if (options.filter != FilterFunction::EWA) {
    Float width = 2 * std::max({std::abs(dstdx[0]), std::abs(dstdx[1]), std::abs(dstdy[0]),
                                std::abs(dstdy[1])});
    int n_levels = levels();
    Float level = n_levels - 1 + log2(std::max<Float>(width, 1e-8));
    if (level >= n_levels - 1) {
      return texel<T>(n_levels - 1, Point2i(0, 0));
    }
    int ilevel = std::max(0, int(pstd::floor(level)));

    if (options.filter == FilterFunction::POINT) {
      Point2i resolution = level_resolution(ilevel);
      Point2i sti(pstd::round(st[0] * resolution[0] - 0.5f),
                  pstd::round(st[1] * resolution[1] - 0.5f));
      return texel<T>(ilevel, sti);
    } else if (options.filter == FilterFunction::BILINEAR) {
      return bilerp<T>(ilevel, st);
    } else {
      DASSERT(options.filter == FilterFunction::TRILINEAR);
      if (ilevel == 0) {
        return bilerp<T>(0, st);
      } else {
        DASSERT_LE(level - ilevel, 1);
        return lerp(level - ilevel, bilerp<T>(ilevel, st), bilerp<T>(ilevel + 1, st));
      }
    }
  }

  if (length_squared(dstdx) < length_squared(dstdy)) {
    pstd::swap(dstdx, dstdy);
  }

  Float longer_vec_length = length(dstdx), shorter_vec_length = length(dstdy);

  if (shorter_vec_length * options.max_anisotropy < longer_vec_length && shorter_vec_length > 0) {
    Float scale = longer_vec_length / (shorter_vec_length * options.max_anisotropy);
    dstdy *= scale;
    shorter_vec_length *= scale;
  }

  if (shorter_vec_length == 0) {
    return bilerp<T>(0, st);
  }

  Float lod = std::max<Float>(0, levels() - 1 + log2(shorter_vec_length));
  int ilod = pstd::floor(lod);
  return lerp(lod - ilod, ewa<T>(ilod, st, dstdx, dstdy), ewa<T>(ilod + 1, st, dstdx, dstdy));
}

template <typename T> T specula::MipMap::texel(int level, Point2i st) const {
  T::unimplemented_function;
}

template <> specula::Float specula::MipMap::texel(int level, Point2i st) const {
  DASSERT(level >= 0 && level < pyramid.size());
  return pyramid[level].get_channel(st, 0, wrap_mode);
}

template <> specula::Rgb specula::MipMap::texel(int level, Point2i st) const {
  DASSERT(level >= 0 && level < pyramid.size());
  if (int nc = pyramid[level].n_channels(); nc == 3 || nc == 4) {
    return {pyramid[level].get_channel(st, 0, wrap_mode),
            pyramid[level].get_channel(st, 1, wrap_mode),
            pyramid[level].get_channel(st, 2, wrap_mode)};
  } else {
    ASSERT_EQ(1, pyramid[level].n_channels());
    Float v = pyramid[level].get_channel(st, 0, wrap_mode);
    return {v, v, v};
  }
}

template <typename T> T specula::MipMap::bilerp(int level, Point2f st) const {
  T::unimplemented_function;
}

template <> specula::Float specula::MipMap::bilerp(int level, Point2f st) const {
  DASSERT(level >= 0 && level < pyramid.size());
  switch (pyramid[level].n_channels()) {
  case 1:
    return pyramid[level].bilerp_channel(st, 0, wrap_mode);
  case 3:
    return pyramid[level].bilerp(st, wrap_mode).average();
  case 5:
    return pyramid[level].bilerp_channel(st, 3, wrap_mode);
  default:
    LOG_CRITICAL("Unexpected number of image channels: {}", pyramid[level].n_channels());
    std::quick_exit(1);
  }
}

template <> specula::Rgb specula::MipMap::bilerp(int level, Point2f st) const {
  DASSERT(level >= 0 && level < pyramid.size());
  if (int nc = pyramid[level].n_channels(); nc == 3 || nc == 4) {
    return {pyramid[level].bilerp_channel(st, 0, wrap_mode),
            pyramid[level].bilerp_channel(st, 1, wrap_mode),
            pyramid[level].bilerp_channel(st, 2, wrap_mode)};
  } else {
    ASSERT_EQ(1, pyramid[level].n_channels());
    Float v = pyramid[level].bilerp_channel(st, 0, wrap_mode);
    return {v, v, v};
  }
}

template <typename T>
T specula::MipMap::ewa(int level, Point2f st, Vector2f dst0, Vector2f dst1) const {
  if (level >= levels()) {
    return texel<T>(levels() - 1, {0, 0});
  }

  Point2i level_res = level_resolution(level);
  st[0] = st[0] * level_res[0] - 0.5f;
  st[1] = st[1] * level_res[1] - 0.5f;
  dst0[0] *= level_res[0];
  dst0[1] *= level_res[1];
  dst1[0] *= level_res[0];
  dst1[1] *= level_res[1];

  Float a = sqr(dst0[1]) + sqr(dst1[1]) + 1;
  Float b = -2 * (dst0[0] * dst0[1] + dst1[0] * dst1[1]);
  Float c = sqr(dst0[0]) + sqr(dst1[0]) + 1;
  Float invf = 1 / (a * c - sqr(b) * 0.25f);
  a *= invf;
  b *= invf;
  c *= invf;

  Float det = -sqr(b) + 4 * a * c;
  Float inv_det = 1 / det;
  Float usqrt = safe_sqrt(det * c), vsqrt = safe_sqrt(a * det);
  int s0 = pstd::ceil(st[0] - 2 * inv_det * usqrt);
  int s1 = pstd::floor(st[0] + 2 * inv_det * usqrt);
  int t0 = pstd::ceil(st[1] - 2 * inv_det * vsqrt);
  int t1 = pstd::floor(st[1] + 2 * inv_det * vsqrt);

  T sum{};
  Float sum_wts = 0;
  for (int it = t0; it <= t1; ++it) {
    Float tt = it - st[1];
    for (int is = s0; is <= s1; ++is) {
      Float ss = is - st[0];
      Float r2 = a * sqr(ss) + b * ss * tt + c * sqr(tt);
      if (r2 < 1) {
        int index = std::min<int>(r2 * MIP_FILTER_LUT_SIZE, MIP_FILTER_LUT_SIZE - 1);
        Float weight = MIP_FILTER_LUT[index];
        sum += weight * texel<T>(level, {is, it});
        sum_wts += weight;
      }
    }
  }

  return sum / sum_wts;
}

specula::pstd::optional<specula::FilterFunction> specula::parse_filter(const char *v) {
  auto val = magic_enum::enum_cast<FilterFunction>(v, magic_enum::case_insensitive);
  if (val.has_value()) {
    return val.value();
  }
  return {};
}

template specula::Float specula::MipMap::filter(Point2f st, Vector2f, Vector2f) const;
template specula::Rgb specula::MipMap::filter(Point2f st, Vector2f, Vector2f) const;
