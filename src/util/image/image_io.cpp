#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>

#include <fmt/ostream.h>
#include <fmt/std.h>

#include "specula/types.hpp"
#include "specula/util/color.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/file.hpp"
#include "specula/util/image/image.hpp"
#include "specula/util/image/image_channel_desc.hpp"
#include "specula/util/image/image_channel_values.hpp"
#include "specula/util/image/image_metadata.hpp"
#include "specula/util/image/pixel_format.hpp"
#include "specula/util/log.hpp"
#include "specula/util/math.hpp"
#include "specula/util/string.hpp"

#ifndef SPECULA_IS_GPU_CODE
#  include <ImfChannelList.h>
#  include <ImfChromaticities.h>
#  include <ImfChromaticitiesAttribute.h>
#  include <ImfFloatAttribute.h>
#  include <ImfFrameBuffer.h>
#  include <ImfHeader.h>
#  include <ImfInputFile.h>
#  include <ImfIntAttribute.h>
#  include <ImfMatrixAttribute.h>
#  include <ImfOutputFile.h>
#  include <ImfPixelType.h>
#  include <ImfStringAttribute.h>
#  include <ImfStringVectorAttribute.h>
#endif

#define LODEPNG_NO_COMPILE_DISK
#include <lodepng.cpp>

#include <lodepng.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_NO_PNG
#define STB_NO_PIC
#define STBI_ASSERT ASSERT
#define STBI_WINDOWS_UTF8

#include <stb_image.h>

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#include <qoi.h>

static constexpr size_t BUFFER_SIZE = 80;
static constexpr bool HOST_LITTLE_ENDIAN =
#if defined(__BYTE_ORDER__)
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    true;
#  elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    false;
#  else
#    error "__BYTE_ORDER__ defined but has unexpected value"
#  endif
#else
#  if defined(__LITTLE_ENDIAN__) || defined(__i386__) || defined(__x86_64__) || defined(_WIN32) || \
      defined(WIN32)
    true;
#  elif defined(__BIG_ENDIAN__)
    false;
#  elif defined(__sparc) || defined(__sparc__)
    false;
#  else
#    error "Cannot detect machine endianness at compile time"
#  endif
#endif

namespace specula {
  static ImageAndMetadata read_exr(const std::filesystem::path &path, Allocator alloc);
  static ImageAndMetadata read_png(const std::filesystem::path &path, Allocator alloc,
                                   ColorEncoding encoding);
  static ImageAndMetadata read_pfm(const std::filesystem::path &path, Allocator alloc);
  static ImageAndMetadata read_hdr(const std::filesystem::path &path, Allocator alloc);
  static ImageAndMetadata read_qoi(const std::filesystem::path &path, Allocator alloc);
} // namespace specula

specula::ImageAndMetadata specula::Image::read(const std::string &filename, Allocator alloc,
                                               ColorEncoding encoding) {
  std::filesystem::path filepath(filename);
  if (filepath.extension() == ".exr") {
    return read_exr(filepath, alloc);
  } else if (filepath.extension() == ".png") {
    return read_png(filepath, alloc, encoding);
  } else if (filepath.extension() == ".pfm") {
    return read_pfm(filepath, alloc);
  } else if (filepath.extension() == ".hdr") {
    return read_hdr(filepath, alloc);
  } else if (filepath.extension() == ".qoi") {
    return read_qoi(filepath, alloc);
  } else {
    int x = 0, y = 0, n = 0;
    unsigned char *data = stbi_load(filename.c_str(), &x, &y, &n, 0);
    if (data == nullptr) {
      LOG_CRITICAL("{} no support for reading images with this extension", filepath);
    }
    pstd::vector<uint8_t> pixels(data, data + static_cast<ptrdiff_t>(x * y * n), alloc);
    stbi_image_free(data);
    switch (n) {
    case 1:
      return ImageAndMetadata{.image = Image(std::move(pixels), {x, y}, {"Y"}, ColorEncoding::SRGB),
                              .metadata = ImageMetadata()};
    case 2: {
      Image image(std::move(pixels), {x, y}, {"Y", "A"}, ColorEncoding::SRGB);
      return ImageAndMetadata{.image = image.select_channels(image.get_channel_desc({"Y"})),
                              .metadata = ImageMetadata()};
    }
    case 3:
      return ImageAndMetadata{
          .image = Image(std::move(pixels), {x, y}, {"R", "G", "B"}, ColorEncoding::SRGB),
          .metadata = ImageMetadata()};
    case 4: {
      Image image(std::move(pixels), {x, y}, {"R", "G", "B", "A"}, ColorEncoding::SRGB);
      return ImageAndMetadata{.image =
                                  image.select_channels(image.get_channel_desc({"R", "G", "B"})),
                              .metadata = ImageMetadata()};
    }
    default:
      LOG_CRITICAL("unsuported {} channel image: {}", n, filepath);
    }
  }

  // TODO: Handle the error messages properly for the quick exit
  std::quick_exit(1);
}

#ifndef SPECULA_IS_GPU_CODE
static Imf::FrameBuffer image_to_frame_buffer(const specula::Image &image,
                                              const specula::ImageChannelDesc &desc,
                                              const Imath::Box2i &data_window) {
  size_t x_stride = image.n_channels() * specula::texel_bytes(image.format());
  size_t y_stride = image.resolution().x * x_stride;

  char *origin_ptr = (((char *)image.raw_pointer({0, 0})) - data_window.min.x * x_stride -
                      data_window.min.y * y_stride);

  Imf::FrameBuffer fb;
  std::vector<std::string> channel_names = image.channel_names();
  switch (image.format()) {
  case specula::PixelFormat::U256:
    LOG_CRITICAL("Unexpected image format: {}", image.format());
  case specula::PixelFormat::HALF:
    for (int channel_index : desc.offset) {
      fb.insert(channel_names[channel_index], Imf::Slice(Imf::FLOAT, origin_ptr + channel_index,
                                                         sizeof(float), x_stride, y_stride));
    }
    break;
  case specula::PixelFormat::FLOAT:
    for (int channel_index : desc.offset) {
      fb.insert(channel_names[channel_index],
                Imf::Slice(Imf::HALF, origin_ptr + channel_index, sizeof(specula::Half), x_stride,
                           y_stride));
    }
    break;
  }
  return fb;
}

specula::ImageAndMetadata specula::read_exr(const std::filesystem::path &path, Allocator alloc) {
  try {
    Imf::InputFile file(path.c_str());
    Imath::Box2i dw = file.header().dataWindow();

    ImageMetadata metadata;
    const auto *render_time_attrib =
        file.header().findTypedAttribute<Imf::FloatAttribute>("renderTimeSeconds");
    if (render_time_attrib != nullptr) {
      metadata.render_time_seconds = render_time_attrib->value();
    }

    const auto *world_to_camera_attrib =
        file.header().findTypedAttribute<Imf::M44fAttribute>("worldToCamera");
    if (world_to_camera_attrib != nullptr) {
      SquareMatrix<4> m;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          m[i][j] = world_to_camera_attrib->value().getValue()[4 * i + j];
        }
      }
      metadata.camera_from_world = m;
    }

    const auto *world_to_ndc_attrib =
        file.header().findTypedAttribute<Imf::M44fAttribute>("worldToNDC");
    if (world_to_ndc_attrib != nullptr) {
      SquareMatrix<4> m;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          m[i][j] = world_to_ndc_attrib->value().getValue()[4 * i + j];
        }
      }
      metadata.ndc_from_world = m;
    }

    metadata.pixel_bounds = {{dw.min.x, dw.min.y}, {dw.max.x + 1, dw.max.y + 1}};

    Imath::Box2i dispw = file.header().displayWindow();
    metadata.full_resolution =
        Point2i(dispw.max.x - dispw.min.x + 1, dispw.max.y - dispw.min.y + 1);

    const auto *spp_attrib = file.header().findTypedAttribute<Imf::IntAttribute>("samplesPerPixel");
    if (spp_attrib != nullptr) {
      metadata.samples_per_pixel = spp_attrib->value();
    }

    const auto *mse_attrib = file.header().findTypedAttribute<Imf::FloatAttribute>("MSE");
    if (mse_attrib != nullptr) {
      metadata.mse = mse_attrib->value();
    }

    for (auto iter = file.header().begin(); iter != file.header().end(); ++iter) {
      if (strcmp(iter.attribute().typeName(), "string") == 0) {
        const auto &sv = dynamic_cast<const Imf::StringAttribute &>(iter.attribute());
        metadata.strings[iter.name()] = sv.value();
      } else if (strcmp(iter.attribute().typeName(), "stringvector") == 0) {
        const auto &sv = dynamic_cast<const Imf::StringVectorAttribute &>(iter.attribute());
        metadata.string_vectors[iter.name()] = sv.value();
      }
    }

    const auto *chromaticities_attrib =
        file.header().findTypedAttribute<Imf::ChromaticitiesAttribute>("chromaticities");
    if (chromaticities_attrib != nullptr) {
      Imf::Chromaticities c = chromaticities_attrib->value();
      const RgbColorSpace *cs =
          RgbColorSpace::lookup(Point2f(c.red.x, c.red.y), Point2f(c.green.x, c.green.y),
                                Point2f(c.blue.x, c.blue.y), Point2f(c.white.x, c.white.y));
      if (cs == nullptr) {
        LOG_WARN("Coudn't find supported color space that matches chromaticities: r ({}, {}), g "
                 "({}, {}), b ({}, {}), w ({}, {}), using sRGB",
                 c.red.x, c.red.y, c.green.x, c.green.y, c.blue.x, c.blue.y, c.white.x, c.white.y);
        metadata.color_space = RgbColorSpace::SRGB;
      } else {
        metadata.color_space = cs;
      }
    }

    int width = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;

    std::vector<std::string> channel_names;
    int n_channels = 0;
    Imf::PixelType pixel_type;
    const Imf::ChannelList &channels = file.header().channels();
    for (auto iter = channels.begin(); iter != channels.end(); ++iter) {
      if (n_channels++ == 0) {
        pixel_type = iter.channel().type;
      } else {
        if (pixel_type != iter.channel().type) {
          LOG_CRITICAL("read_exr() doesn't support images with multiple channel types");
        }
      }
      channel_names.emplace_back(iter.name());
    }

    ASSERT(pixel_type == Imf::HALF || pixel_type == Imf::FLOAT);
    Image image(pixel_type == Imf::HALF ? PixelFormat::HALF : PixelFormat::FLOAT, {width, height},
                channel_names, nullptr, alloc);

    file.setFrameBuffer(image_to_frame_buffer(image, image.all_channels_desc(), dw));
    file.readPixels(dw.min.y, dw.max.y);

    return ImageAndMetadata{.image = std::move(image), .metadata = metadata};
  } catch (const std::exception &err) {
    LOG_CRITICAL("Unabled to read image file \"{}\": {}", path, err.what());
    std::quick_exit(1);
  }

  return {};
}

#endif

specula::ImageAndMetadata specula::read_png(const std::filesystem::path &path, Allocator alloc,
                                            ColorEncoding encoding) {
  std::vector<std::byte> contents = read_file_contents(path);
  if (!encoding) {
    encoding = ColorEncoding::SRGB;
  }

  unsigned width = 0, height = 0;
  LodePNGState state;
  lodepng_state_init(&state);
  unsigned int error = lodepng_inspect(&width, &height, &state,
                                       (const unsigned char *)contents.data(), contents.size());
  if (error != 0) {
    LOG_CRITICAL("{}: {}", path, lodepng_error_text(error));
    std::quick_exit(1);
  }

  Image image(alloc);
  switch (state.info_png.color.colortype) {
  case LCT_GREY:
  case LCT_GREY_ALPHA: {
    std::vector<unsigned char> buf;
    int bpp = state.info_png.color.bitdepth == 16 ? 16 : 8;
    error = lodepng::decode(buf, width, height, (const unsigned char *)contents.data(),
                            contents.size(), LCT_GREY, bpp);
    if (error != 0) {
      LOG_CRITICAL("{}: {}", path, lodepng_error_text(error));
      std::quick_exit(1);
    }

    if (state.info_png.color.bitdepth == 16) {
      image = Image(PixelFormat::HALF, Point2i(width, height), {"Y"});
      auto iter = buf.begin();
      for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x, iter += 2) {
          Float v = (((int)iter[0] << 8) + (int)iter[1]) / 65535.f;
          v = encoding.to_float_linear(v);
          image.set_channel(Point2i(x, y), 0, v);
        }
      }
      DASSERT(iter == buf.end());
    } else {
      image = Image(PixelFormat::U256, Point2i(width, height), {"Y"}, encoding);
      std::ranges::copy(buf, (uint8_t *)image.raw_pointer({0, 0}));
    }
    return ImageAndMetadata{.image = image, .metadata = ImageMetadata()};
  }
  default: {
    std::vector<unsigned char> buf;
    int bpp = state.info_png.color.bitdepth == 16 ? 16 : 8;
    bool has_alpha = (state.info_png.color.colortype == LCT_RGBA);

    error = lodepng::decode(buf, width, height, (const unsigned char *)contents.data(),
                            contents.size(), has_alpha ? LCT_RGBA : LCT_RGB, bpp);
    if (error != 0) {
      LOG_CRITICAL("{}: {}", path, lodepng_error_text(error));
      std::quick_exit(1);
    }

    ImageMetadata metadata;
    metadata.color_space = RgbColorSpace::SRGB;
    if (state.info_png.color.bitdepth == 16) {
      if (has_alpha) {
        image = Image(PixelFormat::HALF, Point2i(width, height), {"R", "G", "B", "A"});
        auto iter = buf.begin();
        for (unsigned int y = 0; y < height; ++y) {
          for (unsigned int x = 0; x < width; ++x, iter += 8) {
            DASSERT(iter < buf.end());
            Float rgba[4] = {
                (((int)iter[0] << 8) + (int)iter[1]) / 65535.f,
                (((int)iter[2] << 8) + (int)iter[3]) / 65535.f,
                (((int)iter[4] << 8) + (int)iter[5]) / 65535.f,
                (((int)iter[6] << 8) + (int)iter[7]) / 65535.f,
            };
            for (int c = 0; c < 4; ++c) {
              rgba[c] = encoding.to_float_linear(rgba[c]);
              image.set_channel(Point2i(x, y), c, rgba[c]);
            }
          }
        }
        DASSERT(iter == buf.end());
      } else {
        image = Image(PixelFormat::HALF, Point2i(width, height), {"R", "G", "B"});
        auto iter = buf.begin();
        for (unsigned int y = 0; y < height; ++y) {
          for (unsigned int x = 0; x < width; ++x, iter += 6) {
            DASSERT(iter < buf.end());
            Float rgb[3] = {
                (((int)iter[0] << 8) + (int)iter[1]) / 65535.f,
                (((int)iter[2] << 8) + (int)iter[3]) / 65535.f,
                (((int)iter[4] << 8) + (int)iter[5]) / 65535.f,
            };
            for (int c = 0; c < 3; ++c) {
              rgb[c] = encoding.to_float_linear(rgb[c]);
              image.set_channel(Point2i(x, y), c, rgb[c]);
            }
          }
        }
        DASSERT(iter == buf.end());
      }
    } else if (has_alpha) {
      image = Image(PixelFormat::U256, Point2i(width, height), {"R", "G", "B", "A"}, encoding);
      std::ranges::copy(buf, (uint8_t *)image.raw_pointer({0, 0}));
    } else {
      image = Image(PixelFormat::U256, Point2i(width, height), {"R", "G", "B"}, encoding);
      std::ranges::copy(buf, (uint8_t *)image.raw_pointer({0, 0}));
    }
    return ImageAndMetadata{.image = image, .metadata = metadata};
  }
  }
}

static inline bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t'; }
static int read_word(FILE *fp, char *buffer, int buffer_length) {

  if (buffer_length < 1) {
    return -1;
  }
  int n = 0, c = fgetc(fp);
  while (c != EOF && is_whitespace(c) && n < buffer_length) {
    buffer[n] = c;
    ++n;
    c = fgetc(fp);
  }

  if (n < buffer_length) {
    buffer[n] = '\0';
    return n;
  }
  return -1;
}

specula::ImageAndMetadata specula::read_pfm(const std::filesystem::path &path, Allocator alloc) {
  pstd::vector<float> rgb32(alloc);
  char buffer[BUFFER_SIZE];
  size_t n_floats;
  int n_channels, width, height;
  float scale;
  bool file_little_endian;
  ImageMetadata metadata;

  FILE *fp = fopen(path.c_str(), "rb");
  if (fp == nullptr) {
    LOG_CRITICAL("{}: unabled to open PFM file", path);
    std::quick_exit(1);
  }
  if (read_word(fp, buffer, BUFFER_SIZE) == -1) {
    LOG_CRITICAL("{}: unable to read PFM file", path);
    std::quick_exit(1);
  }
  if (strcmp(buffer, "Pf") == 0) {
    n_channels = 1;
  } else if (strcmp(buffer, "PF") == 0) {
    n_channels = 3;
  } else {
    LOG_CRITICAL("{}: unabled to decode PFM file type: \"{}{}\"", path, buffer[0], buffer[1]);
    std::quick_exit(1);
  }

  if (read_word(fp, buffer, BUFFER_SIZE) == -1) {
    goto pfm_fail;
  }
  if (!atoi(buffer, &width)) {
    LOG_CRITICAL("{}: unable to decode width \"{}\"", path, buffer);
    std::quick_exit(1);
  }

  if (read_word(fp, buffer, BUFFER_SIZE) == -1) {
    goto pfm_fail;
  }
  if (!atoi(buffer, &height)) {
    LOG_CRITICAL("{}: unable to decode height \"{}\"", path, buffer);
    std::quick_exit(1);
  }

  if (read_word(fp, buffer, BUFFER_SIZE) == -1) {
    goto pfm_fail;
  }
  if (!atof(buffer, &scale)) {
    LOG_CRITICAL("{}: unable to decode scale \"{}\"", path, buffer);
    std::quick_exit(1);
  }

  n_floats = n_channels * size_t(width) * size_t(height);
  rgb32.resize(n_floats);
  for (int y = height - 1; y >= 0; --y) {
    if (fread(&rgb32[n_channels * y * width], sizeof(float), n_channels * width, fp) !=
        n_channels * width) {
      goto pfm_fail;
    }
  }

  file_little_endian = (scale < 0.0f);
  if (HOST_LITTLE_ENDIAN ^ file_little_endian) {
    uint8_t bytes[4];
    for (unsigned int i = 0; i < n_floats; ++i) {
      memcpy(bytes, &rgb32[i], 4);
      pstd::swap(bytes[0], bytes[3]);
      pstd::swap(bytes[1], bytes[2]);
      memcpy(&rgb32[i], bytes, 4);
    }
  }

  if (std::abs(scale) != 1.0f) {
    for (unsigned int i = 0; i < n_floats; ++i) {
      rgb32[i] *= std::abs(scale);
    }
  }
  fclose(fp);
  metadata.color_space = RgbColorSpace::SRGB;
  if (n_channels == 1) {
    return ImageAndMetadata{.image = Image(std::move(rgb32), {width, height}, {"Y"}),
                            .metadata = metadata};
  } else {
    return ImageAndMetadata{.image = Image(std::move(rgb32), {width, height}, {"R", "G", "B"}),
                            .metadata = metadata};
  }

pfm_fail:
  if (fp != nullptr) {
    fclose(fp);
  }
  LOG_CRITICAL("{}: premature end of file in PFM file", path);
  std::quick_exit(1);
}

specula::ImageAndMetadata specula::read_hdr(const std::filesystem::path &path, Allocator alloc) {
  int x = 0, y = 0, n = 0;
  float *data = stbi_loadf(path.c_str(), &x, &y, &n, 0);
  if (data == nullptr) {
    LOG_CRITICAL("{}: {}", path, stbi_failure_reason());
    std::quick_exit(1);
  }

  pstd::vector<float> pixels(data, data + static_cast<ptrdiff_t>(x * y * n), alloc);
  stbi_image_free(data);

  switch (n) {
  case 1:
    return ImageAndMetadata{.image = Image(std::move(pixels), {x, y}, {"Y"}),
                            .metadata = ImageMetadata()};
  case 2: {
    Image image(std::move(pixels), {x, y}, {"Y", "A"});
    return ImageAndMetadata{.image = image.select_channels(image.get_channel_desc({"Y"})),
                            .metadata = ImageMetadata()};
  }
  case 3:
    return ImageAndMetadata{.image = Image(std::move(pixels), {x, y}, {"R", "G", "B"}),
                            .metadata = ImageMetadata()};
  case 4: {
    Image image(std::move(pixels), {x, y}, {"R", "G", "B", "A"});
    return ImageAndMetadata{.image = image.select_channels(image.get_channel_desc({"R", "G", "B"})),
                            .metadata = ImageMetadata()};
  }
  default:
    LOG_CRITICAL("unsuported {} channel image: {}", n, path);
    std::quick_exit(1);
  }
}

specula::ImageAndMetadata specula::read_qoi(const std::filesystem::path &path, Allocator alloc) {
  std::vector<std::byte> contents = read_file_contents(path);
  qoi_desc desc;
  void *pixels = qoi_decode(contents.data(), contents.size(), &desc, 0);
  ASSERT(pixels != nullptr);

  ImageMetadata metadata;
  metadata.color_space = RgbColorSpace::SRGB;

  std::vector<std::string> channel_names{"R", "G", "B"};
  if (desc.channels == 4) {
    channel_names.emplace_back("A");
  } else {
    ASSERT_EQ(3, desc.channels);
  }

  ASSERT(desc.colorspace == QOI_SRGB || desc.colorspace == QOI_LINEAR);
  ColorEncoding encoding =
      (desc.colorspace == QOI_SRGB) ? ColorEncoding::SRGB : ColorEncoding::LINEAR;

  Image image(PixelFormat::U256, Point2i(desc.width, desc.height), channel_names, encoding, alloc);
  std::memcpy(image.raw_pointer({0, 0}), pixels,
              size_t(desc.width) * size_t(desc.height) * desc.channels);
  free(pixels);
  return ImageAndMetadata{.image = image, .metadata = metadata};
}

bool specula::Image::write(const std::string &filename, const ImageMetadata &metadata) const {
  std::filesystem::path filepath(filename);

  if (metadata.pixel_bounds) {
    ASSERT_EQ(metadata.pixel_bounds->area(), size_t(resolution_.x) * size_t(resolution_.y));
  }
  if (filepath.extension() == ".exr") {
    return write_exr(filepath, metadata);
  }

  Image out_image = *this;
  switch (n_channels()) {
  case 1:
    break;
  case 2:
    LOG_ERROR("{}: unable to write a 2 channel image in this format", filepath);
    return false;
  case 3: {
    ImageChannelDesc desc = out_image.get_channel_desc({"R", "G", "B"});
    if (!desc) {
      LOG_WARN("{}: image has 3 channels but they are not R, G, and B. Image may be garbled",
               filepath);
    } else {
      out_image = out_image.select_channels(desc);
    }
    break;
  }
  case 4: {
    ImageChannelDesc desc = out_image.get_channel_desc({"R", "G", "B", "A"});
    if (!desc) {
      LOG_WARN("{}: image has 4 channels but they are not R, G, B and A. Image may be garbled",
               filepath);
    } else {
      out_image = out_image.select_channels(desc);
    }
    break;
  }
  default: {
    ImageChannelDesc desc = out_image.get_channel_desc({"R", "G", "B", "A"});
    if (!desc) {
      LOG_ERROR("{}: multi-channel image does not have R, G, and B. Unable to write to this format",
                filepath);
      return false;
    } else {
      std::vector<std::string> ignored_channels;
      for (const auto &ch : out_image.channel_names()) {
        if (ch != "R" && ch != "G" && ch != "B") {
          ignored_channels.push_back(ch);
        }
      }
      LOG_WARN("{}: ignoring additional channels for this image format: {}", filepath,
               ignored_channels);
    }

    out_image = out_image.select_channels(desc);
    break;
  }
  }

  ASSERT(out_image.n_channels() == 1 || out_image.n_channels() == 3 || out_image.n_channels() == 4);

  ImageMetadata out_metadata = metadata;
  if (out_image.n_channels() != 1 && *metadata.get_color_space() != *RgbColorSpace::SRGB) {
    LOG_WARN("{}: converting pixel colors to sRGB to match output image format", filepath);
    SquareMatrix<3> m = convert_rgb_color_space(*metadata.get_color_space(), *RgbColorSpace::SRGB);
    ImageChannelDesc rgb_desc = out_image.get_channel_desc({"R", "G", "B"});
    if (rgb_desc) {
      for (int y = 0; y < out_image.resolution_.y; ++y) {
        for (int x = 0; x < out_image.resolution_.x; ++x) {
          ImageChannelValues channels = out_image.get_channels({x, y}, rgb_desc);
          Rgb rgb = mul<Rgb>(m, channels);
          out_image.set_channels({x, y}, rgb_desc, {rgb.r, rgb.g, rgb.b});
        }
      }
      out_metadata.color_space = RgbColorSpace::SRGB;
    }
  }

  if (filepath.extension() == ".pfm") {
    return out_image.write_pfm(filepath, out_metadata);
  } else if (filepath.extension() == ".png") {
    return out_image.write_png(filepath, out_metadata);
  } else if (filepath.extension() == ".qoi") {
    return out_image.write_qoi(filepath, out_metadata);
  } else {
    LOG_ERROR("{}: no support for writing images with this extension", filepath);
    return false;
  }
}

bool specula::Image::write_exr(const std::filesystem::path &path,
                               const ImageMetadata &metadata) const {
  if (is_8bit(format_)) {
    return convert_to_format(PixelFormat::HALF).write_exr(path, metadata);
  }
  ASSERT(is_16bit(format_) || is_32bit(format_));

  try {
    Imath::Box2i display_window, data_window;
    if (metadata.full_resolution) {
      display_window = {Imath::V2i(0, 0), Imath::V2i(metadata.full_resolution->x - 1,
                                                     metadata.full_resolution->y - 1)};
    } else {
      display_window = {Imath::V2i(0, 0), Imath::V2i(resolution_.x - 1, resolution_.y - 1)};
    }

    if (metadata.pixel_bounds) {
      data_window = {
          Imath::V2i(metadata.pixel_bounds->p_min.x, metadata.pixel_bounds->p_min.y),
          Imath::V2i(metadata.pixel_bounds->p_max.x - 1, metadata.pixel_bounds->p_max.y - 1)};
    } else {
      data_window = {Imath::V2i(0, 0), Imath::V2i(resolution_.x - 1, resolution_.y - 1)};
    }

    Imf::FrameBuffer fb = image_to_frame_buffer(*this, all_channels_desc(), data_window);

    Imf::Header header(display_window, data_window);
    for (auto iter = fb.begin(); iter != fb.end(); ++iter) {
      header.channels().insert(iter.name(), iter.slice().type);
    }

    if (metadata.render_time_seconds) {
      header.insert("renderTimeSeconds", Imf::FloatAttribute(*metadata.render_time_seconds));
    }

    if (metadata.camera_from_world) {
      float m[4][4];
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          m[i][j] = (*metadata.camera_from_world)[i][j];
        }
      }
      header.insert("worldToCamera", Imf::M44fAttribute(m));
    }

    if (metadata.ndc_from_world) {
      float m[4][4];
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          m[i][j] = (*metadata.ndc_from_world)[i][j];
        }
      }
      header.insert("worldToNDC", Imf::M44fAttribute(m));
    }

    if (metadata.samples_per_pixel) {
      header.insert("samplesPerPixel", Imf::IntAttribute(*metadata.samples_per_pixel));
    }
    if (metadata.mse) {
      header.insert("MSE", Imf::FloatAttribute(*metadata.mse));
    }
    for (const auto &iter : metadata.strings) {
      header.insert(iter.first, Imf::StringAttribute(iter.second));
    }
    for (const auto &iter : metadata.string_vectors) {
      header.insert(iter.first, Imf::StringVectorAttribute(iter.second));
    }

    if (*metadata.get_color_space() != *RgbColorSpace::SRGB) {
      const RgbColorSpace &cs = *metadata.get_color_space();
      Imf::Chromaticities chromaticities(Imath::V2f(cs.r.x, cs.r.y), Imath::V2f(cs.g.x, cs.g.y),
                                         Imath::V2f(cs.b.x, cs.b.y), Imath::V2f(cs.w.x, cs.w.y));
      header.insert("chromaticities", Imf::ChromaticitiesAttribute(chromaticities));
    }

    Imf::OutputFile file(path.c_str(), header);
    file.setFrameBuffer(fb);
    file.writePixels(resolution_.y);
  } catch (const std::exception &err) {
    LOG_ERROR("{}: error writing EXR: {}", path, err.what());
    return false;
  }
  return true;
}

bool specula::Image::write_pfm(const std::filesystem::path &path,
                               const ImageMetadata &metadata) const {
  std::ofstream ofs(path, std::ios::out | std::ios::binary);
  if (!ofs) {
    LOG_ERROR("{}: {}", path, std::strerror(errno));
    return false;
  }
  if (n_channels() != 3) {
    LOG_ERROR("{}: only 3-channel images are supported for PFM");
    return false;
  }

  std::unique_ptr<float[]> scanline = std::make_unique<float[]>(3 * resolution_.x);
  float scale = NAN;

  fmt::println(ofs, "PF");
  fmt::println(ofs, "{} {}", resolution_.x, resolution_.y);
  scale = HOST_LITTLE_ENDIAN ? -1.0f : 1.0f;
  fmt::println(ofs, "{}", scale);

  for (int y = resolution_.y - 1; y >= 0; y--) {
    for (int x = 0; x < resolution_.x; ++x) {
      if (n_channels() == 1) {
        Float v = get_channel({x, y}, 0);
        scanline[3 * x] = scanline[3 * x + 1] = scanline[3 * x + 2] = v;
      } else {
        ASSERT_EQ(3, n_channels());
        for (int c = 0; c < 3; ++c) {
          scanline[3 * x + c] = get_channel({x, y}, c);
        }
      }
    }

    ofs.write((const char *)&scanline[0], sizeof(float) * 3 * resolution_.x);
  }

  ofs.close();
  return true;
}

bool specula::Image::write_png(const std::filesystem::path &path,
                               const ImageMetadata &metadata) const {
  unsigned int error = 0;
  int n_out_of_gamut = 0;

  unsigned char *png;
  size_t png_size;

  LodePNGColorType png_color;
  switch (n_channels()) {
  case 1:
    png_color = LCT_GREY;
    break;
  case 3:
    // TODO: store the color encoding used in the PNG metadata
    png_color = LCT_RGB;
    break;
  case 4:
    png_color = LCT_RGBA;
    break;
  default:
    LOG_ERROR("Unexpected number of channels in write_png()");
    return false;
  }

  if (format_ == PixelFormat::U256) {
    error = lodepng_encode_memory(&png, &png_size, p8.data(), resolution_.x, resolution_.y,
                                  png_color, 8);
  } else {
    std::unique_ptr<uint8_t[]> pix8 = quantize_pixels_to_u256(&n_out_of_gamut);
    error = lodepng_encode_memory(&png, &png_size, pix8.get(), resolution_.x, resolution_.y,
                                  png_color, 8);
  }

  if (n_out_of_gamut > 0) {
    LOG_WARN("{}: {} out of gamut pixel channels clamped to [0,1]", path, n_out_of_gamut);
  }

  if (error == 0) {
    if (!write_file_contents(path, (std::byte *)png, png_size)) {
      LOG_ERROR("{}: error writing PNG", path);
      free(png);
      return false;
    }
  } else {
    LOG_ERROR("{}: {}", path, lodepng_error_text(error));
    free(png);
    return false;
  }
  free(png);
  return true;
}

bool specula::Image::write_qoi(const std::filesystem::path &path,
                               const ImageMetadata &metadata) const {
  Image image = *this;
  void *qoi_pixels = nullptr;
  int qoi_size = 0;

  qoi_desc desc;
  desc.width = resolution_.x;
  desc.height = resolution_.y;
  desc.channels = n_channels();
  if (encoding_ && !encoding_.is<LinearColorEncoding>() && !encoding_.is<sRgbColorEncoding>()) {
    LOG_ERROR("{}: only linear and sRGB encodings are supported by QOI", encoding_);
    return false;
  }
  desc.colorspace = encoding_.is<LinearColorEncoding>() ? QOI_LINEAR : QOI_SRGB;

  if (n_channels() == 4) {
    ImageChannelDesc desc = get_channel_desc({"R", "G", "B", "A"});
    if (desc) {
      image = select_channels(desc);
    }
  } else if (n_channels() == 3) {
    ImageChannelDesc desc = get_channel_desc({"R", "G", "B"});
    if (desc) {
      image = select_channels(desc);
    }
  } else {
    LOG_ERROR("{}: only 3 and 4 channel images are supported for QOI", path);
    return false;
  }

  if (format_ == PixelFormat::U256) {
    qoi_pixels = qoi_encode(image.raw_pointer({0, 0}), &desc, &qoi_size);
  } else {
    int n_out_of_gamut = 0;
    std::unique_ptr<uint8_t[]> rgba8 = quantize_pixels_to_u256(&n_out_of_gamut);
    if (n_out_of_gamut > 0) {
      LOG_WARN("{}: {} out of gamut pixel channels clamped to [0,1]", path, n_out_of_gamut);
    }
    qoi_pixels = qoi_encode(rgba8.get(), &desc, &qoi_size);
  }

  bool success = write_file_contents(path, (std::byte *)qoi_pixels, qoi_size);
  if (!success) {
    LOG_ERROR("{}: error writing QOI file", path);
  }

  free(qoi_pixels);
  return success;
}
