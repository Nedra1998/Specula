#ifndef INCLUDE_IMAGE_IMAGE_HPP_
#define INCLUDE_IMAGE_IMAGE_HPP_

#include <string>

#include "specula.hpp"
#include "util/check.hpp"
#include "util/color/color_encoding.hpp"
#include "util/containers/array2d.hpp"
#include "util/float.hpp"
#include "util/image/image_channel.hpp"
#include "util/image/metadata.hpp"
#include "util/image/pixel_format.hpp"
#include "util/image/wrap_mode.hpp"
#include "util/log.hpp"
#include "util/pstd/vector.hpp"
#include "util/vecmath/bounds2.hpp"
#include "util/vecmath/tuple2.hpp"

namespace specula {
  struct ImageAndMetadata;

  struct ResampleWeight {
    int first_pixel;
    Float weight[4];
  };

  class Image {
  public:
    Image(Allocator alloc = {})
        : p8(alloc), p16(alloc), p32(alloc), format_(PixelFormat::U256), resolution_(0, 0) {}

    Image(pstd::vector<uint8_t> p8, Point2i resolution, pstd::span<const std::string> channels,
          ColorEncoding encoding);
    Image(pstd::vector<Half> p16, Point2i resolution, pstd::span<const std::string> channels);
    Image(pstd::vector<float> p32, Point2i resolution, pstd::span<const std::string> channels);

    Image(PixelFormat format, Point2i resolution, pstd::span<const std::string> channels,
          ColorEncoding encoding = nullptr, Allocator alloc = {});

    static pstd::vector<Image> generate_pyramid(Image image, WrapMode2D wrap_mode,
                                                Allocator alloc = {});

    SPECULA_CPU_GPU PixelFormat format() const { return format_; }
    SPECULA_CPU_GPU Point2i resolution() const { return resolution_; }
    SPECULA_CPU_GPU int n_channels() const { return channel_names_.size(); }
    std::vector<std::string> channel_names() const;
    std::vector<std::string> channel_names(const ImageChannelDesc &) const;
    const ColorEncoding encoding() const { return encoding_; }

    SPECULA_CPU_GPU size_t pixel_offset(Point2i p) const {
      DASSERT(inside_exclusive(p, Bounds2i({0, 0}, resolution)));
      return n_channels() * (p.y * resolution_.x + p.x);
    }

    SPECULA_CPU_GPU Float get_channel(Point2i p, int c,
                                      WrapMode2D wrap_mode = WrapMode::Clamp) const {
      if (!remap_pixel_coords(&p, resolution_, wrap_mode)) {
        return 0;
      }

      switch (format_) {
      case PixelFormat::U256: {
        Float r;
        encoding_.to_linear({&p8[pixel_offset(p) + c], 1}, {&r, 1});
        return r;
      }
      case PixelFormat::Half: {
        return Float(p16[pixel_offset(p) + c]);
      }
      case PixelFormat::Float: {
        return p32[pixel_offset(p) + c];
      }
      default:
        LOG_CRITICAL("Unhandled PixelFormat: {}", format_);
        return 0;
      }
    }

    SPECULA_CPU_GPU Float bilerp_channel(Point2f p, int c,
                                         WrapMode2D wrap_mode = WrapMode::Clamp) const {
      Float x = p[0] * resolution_.x - 0.5f, y = p[1] * resolution_.y - 0.05f;
      int xi = pstd::floor(x), yi = pstd::floor(y);
      Float dx = x - xi, dy = y - yi;

      pstd::array<Float, 4> v = {
          get_channel({xi, yi}, c, wrap_mode),
          get_channel({xi + 1, yi}, c, wrap_mode),
          get_channel({xi, yi + 1}, c, wrap_mode),
          get_channel({xi + 1, yi + 1}, c, wrap_mode),
      };

      return ((1 - dx) * (1 - dy) * v[0] + dx * (1 - dy) * v[1] + (1 - dx) * dy * v[2] +
              dx * dy * v[3]);
    }

    SPECULA_CPU_GPU inline void set_channel(Point2i p, int c, Float value) {
      if (isnan(value)) {
#ifndef SPECULA_IS_GPU_CODE
        LOG_ERROR("NaN at pixel {},{} comp {}", p.x, p.y, c);
#endif
        value = 0;
      }

      switch (format_) {
      case PixelFormat::U256:
        encoding_.from_linear({&value, 1}, {&p8[pixel_offset(p) + c], 1});
        break;
      case PixelFormat::Half:
        p16[pixel_offset(p) + c] = Half(value);
        break;
      case PixelFormat::Float:
        p32[pixel_offset(p) + c] = value;
        break;
      default:
        LOG_CRITICAL("Unhandled PixelFormat: {}", format_);
      }
    }

    ImageChannelDesc get_channel_desc(pstd::span<const std::string> channels) const;
    ImageChannelDesc all_channels_desc() const {
      ImageChannelDesc desc;
      desc.offset.resize(n_channels());
      for (int i = 0; i < n_channels(); ++i) {
        desc.offset[i] = i;
      }
      return desc;
    }

    ImageChannelValues get_channels(Point2i p, WrapMode2D wrap_mode = WrapMode::Clamp) const;
    ImageChannelValues get_channels(Point2i p, const ImageChannelDesc &desc,
                                    WrapMode2D wrap_mode = WrapMode::Clamp) const;

    Image select_channels(const ImageChannelDesc &desc, Allocator alloc = {}) const;
    Image crop(const Bounds2i &bounds, Allocator alloc = {}) const;

    void copy_rect_out(const Bounds2i &extent, pstd::span<float> buf,
                       WrapMode2D wrap_mode = WrapMode::Clamp) const;
    void copy_rect_in(const Bounds2i &extent, pstd::span<const float> buf);

    ImageChannelValues avarge(const ImageChannelDesc &desc) const;

    bool has_any_infinite_pixels() const;
    bool has_any_nan_pixels() const;

    ImageChannelValues mae(const ImageChannelDesc &desc, const Image &ref,
                           Image *error_image = nullptr) const;
    ImageChannelValues mse(const ImageChannelDesc &desc, const Image &ref,
                           Image *error_image = nullptr) const;
    ImageChannelValues mrse(const ImageChannelDesc &desc, const Image &ref,
                            Image *error_image = nullptr) const;

    Image gaussian_filter(const ImageChannelDesc &desc, int half_width, Float sigma) const;

    template <typename F>
    Array2D<Float> get_sampling_distribution(F dxda,
                                             const Bounds2f &domain = Bounds2f(Point2f(0, 0),
                                                                               Point2f(1, 1)),
                                             Allocator alloc = {}) {
      Array2D<Float> dist(resolution_[0], resolution_[1], alloc);
      // TODO: Uncomment when parallel processing has been implemented
      //
      // parallel_for(0, resolution_[1], [&](int64_t y0, int64_t y1) {
      //   for (int y = y0; y < y1; ++y) {
      //     for (int x = 0; x < resolution_[0]; ++x) {
      //       Float value = get_channels({x, y}).average();
      //
      //       Point2f p =
      //           domain.lerp(Point2f((x + 0.5f) / resolution_[0], (y + 0.5f) / resolution_[1]));
      //       dist(x, y) = value * dxda(p);
      //     }
      //   }
      // });
      return dist;
    }

    Array2D<Float> get_sampling_distribution() {
      return get_sampling_distribution([](Point2f) { return Float(1); });
    }

    static ImageAndMetadata read(std::string filename, Allocator alloc = {},
                                 ColorEncoding encoding = nullptr);
    bool write(std::string name, const ImageMetadata &metadata = {}) const;

    Image convert_to_format(PixelFormat format, ColorEncoding encoding = nullptr) const;

    SPECULA_CPU_GPU Float lookup_nearest_channel(Point2f p, int c,
                                                 WrapMode2D wrap_mode = WrapMode::Clamp) const {
      Point2i pi(p.x * resolution_.x, p.y * resolution_.y);
      return get_channel(pi, c, wrap_mode);
    }

    ImageChannelValues lookup_nearest(Point2f p, WrapMode2D wrap_mode = WrapMode::Clamp) const;
    ImageChannelValues lookup_nearest(Point2f p, const ImageChannelDesc &desc,
                                      WrapMode2D wrap_mode = WrapMode::Clamp) const;

    ImageChannelValues bilerp(Point2f p, WrapMode2D wrap_mode = WrapMode::Clamp) const;
    ImageChannelValues bilerp(Point2f p, const ImageChannelDesc &desc,
                              WrapMode2D wrap_mode = WrapMode::Clamp) const;

    void set_channels(Point2i p, const ImageChannelValues &values);
    void set_channels(Point2i p, pstd::span<const Float> values);
    void set_channels(Point2i p, const ImageChannelDesc &desc, pstd::span<const Float> values);

    Image float_resize_up(Point2i new_resolution, WrapMode2D wrap) const;

    void flip_y();

    SPECULA_CPU_GPU size_t bytes_used() const {
      return p8.size() + 2 * p16.size() + 4 * p32.size();
    }

    SPECULA_CPU_GPU const void *raw_pointer(Point2i p) const {
      if (is_8bit(format_)) {
        return p8.data() + pixel_offset(p);
      } else if (is_16bit(format_)) {
        return p16.data() + pixel_offset(p);
      } else {
        ASSERT(is_32bit(format_));
        return p32.data() + pixel_offset(p);
      }
    }

    SPECULA_CPU_GPU void *raw_pointer(Point2i p) {
      return const_cast<void *>(((const Image *)this)->raw_pointer(p));
    }

    Image joint_bilateral_filter(const ImageChannelDesc &to_filter, int half_width,
                                 const Float xy_sigma[2], const ImageChannelDesc &joint,
                                 const ImageChannelValues &joint_sigma) const;

    SPECULA_CPU_GPU operator bool() const { return resolution_.x > 0 && resolution_.y > 0; }

  private:
    static std::vector<ResampleWeight> resample_weights(int old_res, int new_res);

    bool write_exr(const std::string &name, const ImageMetadata &metadata) const;
    bool write_pfm(const std::string &name, const ImageMetadata &metadata) const;
    bool write_png(const std::string &name, const ImageMetadata &metadata) const;
    bool write_qoi(const std::string &name, const ImageMetadata &metadata) const;

    std::unique_ptr<uint8_t[]> quantize_pixels_to_u256(int *n_out_of_gamut) const;

    PixelFormat format_;
    Point2i resolution_;
    pstd::vector<std::string> channel_names_;
    ColorEncoding encoding_ = nullptr;

    pstd::vector<uint8_t> p8;
    pstd::vector<Half> p16;
    pstd::vector<float> p32;
  };

  struct ImageAndMetadata {
    Image image;
    ImageMetadata metadata;
  };
} // namespace specula

#endif // INCLUDE_IMAGE_IMAGE_HPP_
