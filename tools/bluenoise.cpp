#include <cassert>
#include <fstream>
#include <ios>
#include <vector>

#include <cxxopts.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include "lodepng.cpp"
#include "lodepng.h"

#if defined(_WIN32)
#  include <io.h>
#  define IS_STDERR_TTY() (_isatty(_fileno(stderr)) != 0)
#else
#  include <unistd.h>
#  define IS_STDERR_TTY() (isatty(fileno(stderr)) != 0)
#endif

std::vector<uint8_t> read_file(const std::string &file) {
  spdlog::debug("Reading input file '{}'", file);
  std::ifstream in(file, std::ios::in | std::ios::binary);
  if (!in.is_open()) {
    spdlog::error("Failed to open file: '{}'", file);
    exit(1);
  }

  in.seekg(0, std::ios::end);
  std::streamsize length = in.tellg();
  in.seekg(0, std::ios::beg);

  std::vector<uint8_t> buf(length);
  in.read(reinterpret_cast<char *>(buf.data()), length);
  in.close();

  spdlog::debug("Read {} bytes from '{}'", buf.size(), file);
  return buf;
}

std::array<std::vector<std::vector<uint16_t>>, 3> read_png(const std::string &file, unsigned *xres,
                                                           unsigned *yres) {
  spdlog::debug("Inspecting PNG '{}'", file);

  // Read the file contents into memory
  std::vector<uint8_t> bytes = read_file(file);

  LodePNGState state;
  lodepng_state_init(&state);
  unsigned int error = lodepng_inspect(xres, yres, &state, bytes.data(), bytes.size());

  if (error != 0) {
    spdlog::error("LodePNG Error ({}): {}", file, lodepng_error_text(error));
    exit(1);
  }

  if (state.info_png.color.colortype == LCT_GREY ||
      state.info_png.color.colortype == LCT_GREY_ALPHA) {
    spdlog::error("PNG '{}' color type must not be LCT_GREY or LCT_GREY_ALPHA", file);
    exit(1);
  }

  unsigned bpp = state.info_png.color.bitdepth == 16 ? 16 : 8;

  // Decode the image into memory
  std::vector<unsigned char> data;
  error = lodepng::decode(data, *xres, *yres, bytes.data(), bytes.size(), LCT_RGB, bpp);
  if (error != 0) {
    spdlog::error("LodePNG Error ({}): {}", file, lodepng_error_text(error));
    exit(1);
  }

  spdlog::debug("Decoded '{}' as {}x{} {}-bit RGB", file, *xres, *yres, bpp);

  // Map the pixel data into the output object
  std::array<std::vector<std::vector<uint16_t>>, 3> output = {
      std::vector<std::vector<uint16_t>>(*yres, std::vector<uint16_t>(*xres)),
      std::vector<std::vector<uint16_t>>(*yres, std::vector<uint16_t>(*xres)),
      std::vector<std::vector<uint16_t>>(*yres, std::vector<uint16_t>(*xres)),
  };

  if (bpp == 16) {
    auto iter = data.begin();
    for (unsigned int y = 0; y < *yres; ++y) {
      for (unsigned int x = 0; x < *xres; ++x, iter += 6) {
        assert(iter < data.end());
        output[0][y][x] = ((uint16_t)iter[0] << 8) + (uint16_t)iter[1];
        output[1][y][x] = ((uint16_t)iter[2] << 8) + (uint16_t)iter[3];
        output[2][y][x] = ((uint16_t)iter[4] << 8) + (uint16_t)iter[5];
      }
    }
    assert(iter == data.end());
  } else {
    auto iter = data.begin();
    for (unsigned int y = 0; y < *yres; ++y) {
      for (unsigned int x = 0; x < *xres; ++x, iter += 6) {
        assert(iter < data.end());
        output[0][y][x] = (uint16_t)iter[0];
        output[1][y][x] = (uint16_t)iter[1];
        output[2][y][x] = (uint16_t)iter[2];
      }
    }
    assert(iter == data.end());
  }

  return output;
}

int main(int argc, const char *argv[]) {
  try {
    cxxopts::Options options(
        "bluenoise", "Read RGB images and embed them component-wise into a C++ source file");

    // clang-format off
    options.add_options("General")
      ("h,help", "Print this help message and exit")
      ("v,verbose", "Enable debug/verbose logging", cxxopts::value<bool>())
      ("q,quiet", "Suppress non-error logging", cxxopts::value<bool>())
    ;

    options.add_options()
      ("images", "The PNG images to embed", cxxopts::value<std::vector<std::string>>())
      ("o,output", "The output path of the generated C++", cxxopts::value<std::string>()->default_value("bluenoise.cpp"))
    ;

    options.positional_help("IMAGE...");
    options.parse_positional({"images"});
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.contains("help")) {
      fmt::print("{}\n", options.help());
      return 0;
    }

    std::shared_ptr<spdlog::sinks::sink> sink = nullptr;
    bool is_tty = IS_STDERR_TTY();
    if (is_tty) {
      sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      sink->set_pattern("\033[90m[%H:%M:%S.%e]\033[0m \033[1m%^%-8l%$\033[0m  %v");
    } else {
      sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
      sink->set_pattern("[%H:%M:%S.%e] [%-8l] %v");
    }

    auto logger = std::make_shared<spdlog::logger>("tool", sink);
    if (result.contains("quiet")) {
      logger->set_level(spdlog::level::err);
    } else if (result.contains("verbose")) {
      logger->set_level(spdlog::level::debug);
    } else if (!is_tty) {
      logger->set_level(spdlog::level::warn);
    } else {
      logger->set_level(spdlog::level::info);
    }
    spdlog::set_default_logger(logger);

    auto images = result["images"].as<std::vector<std::string>>();
    auto output = result["output"].as<std::string>();

    spdlog::info("Preparing to embed {} image{} into '{}'", images.size(),
                 images.size() == 1 ? "" : "s", output);

    if (images.empty()) {
      spdlog::error("At least one PNG image is required to embed");
      return 1;
    }

    // Open the output file for writing
    std::ofstream fout(output);
    if (!fout.is_open()) {
      spdlog::error("Could not create output file: '{}'", output);
      return 1;
    }

    // Write the standard header information
    fmt::println(fout, "#include \"specula/macros.hpp\"");
    fmt::println(fout, "#include <cstdint>\n");
    fmt::println(fout, "namespace specula {{");
    fmt::print(fout, "SPECULA_CONST uint16_t BLUE_NOISE_TEXTURE[{}]", images.size() * 3);

    // Read and embed each image
    unsigned xres = 0, yres = 0;
    for (const auto &file : images) {
      spdlog::debug("Processing image '{}'", file);
      unsigned img_xres = 0, img_yres = 0;
      std::array<std::vector<std::vector<uint16_t>>, 3> data = read_png(file, &img_xres, &img_yres);

      // Check the image resolution stays consistent
      if (xres == 0) {
        xres = img_xres;
        yres = img_yres;
        spdlog::debug("Using {}x{} as the expected image resolution", xres, yres);

        // Finish writing the header
        fmt::println(fout, "[{}][{}] = {{", xres, yres);

      } else if (xres != img_xres || yres != img_yres) {
        spdlog::error("Image '{}' has a different resolution ({}x{}) than the expected ({}x{}), "
                      "only one resolution is permited",
                      file, img_xres, img_yres, xres, yres);
        return 1;
      }

      // Write the pixel data
      for (const auto &channel : data) {
        fmt::println(fout, "    {{");
        for (const auto &row : channel) {
          fmt::println(fout, "        {{ {:5} }},", fmt::join(row, ", "));
        }
        fmt::println(fout, "    }},");
      }
    }

    // Write closing brackets
    fmt::println(fout, "}};\n}}");
    fout.close();
    if (fout.fail()) {
      spdlog::error("Failed while finalizing output file '{}'", output);
      return 1;
    }

    spdlog::info("Generated '{}' successfully ({} images, {}x{})", output, images.size(), xres,
                 yres);

  } catch (const std::exception &e) {
    fmt::print(stderr, "CLI Parsing Error: {}\n", e.what());
    exit(1);
  }
}
