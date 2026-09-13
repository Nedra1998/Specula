#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>

#include <cxxopts.hpp>

#include "lodepng.cpp"
#include "lodepng.h"

std::string read_file_contents(const std::string &filename) {
  std::ifstream ifs(filename, std::ios::binary);
  if (!ifs) {
    std::cerr << filename << ": " << strerror(errno) << '\n';
    exit(1);
  }
  return {(std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>())};
}

std::vector<uint16_t> read_png(const std::string &filename, unsigned int *xres,
                               unsigned int *yres) {
  std::string contents = read_file_contents(filename);
  unsigned width = 0, height = 0;
  LodePNGState state;
  lodepng_state_init(&state);
  unsigned int error = lodepng_inspect(&width, &height, &state,
                                       (const unsigned char *)contents.data(), contents.size());
  if (error != 0) {
    std::cerr << filename << ": " << lodepng_error_text(error) << '\n';
    exit(1);
  }

  *xres = width;
  *yres = height;

  assert(state.info_png.color.colortype != LCT_GREY &&
         state.info_png.color.colortype != LCT_GREY_ALPHA);

  int bpp = state.info_png.color.bitdepth == 16 ? 16 : 8;
  std::vector<unsigned char> buf;
  error = lodepng::decode(buf, width, height, (const unsigned char *)contents.data(),
                          contents.size(), LCT_RGB, bpp);
  if (error != 0) {
    std::cerr << filename << ": " << lodepng_error_text(error) << '\n';
    exit(1);
  }

  std::vector<uint16_t> pix;
  if (state.info_png.color.bitdepth == 16) {
    auto iter = buf.begin();
    for (unsigned int y = 0; y < height; ++y) {
      for (unsigned int x = 0; x < width; ++x, iter += 6) {
        assert(iter < buf.end());
        pix.push_back(((int)iter[0] << 8) + (int)iter[1]);
        pix.push_back(((int)iter[2] << 8) + (int)iter[3]);
        pix.push_back(((int)iter[4] << 8) + (int)iter[5]);
      }
    }
    assert(iter == buf.end());
  } else {
    auto iter = buf.begin();
    for (unsigned int y = 0; y < height; ++y) {
      for (unsigned int x = 0; x < width; ++x, iter += 3) {
        assert(iter < buf.end());
        pix.push_back((int)iter[0]);
        pix.push_back((int)iter[1]);
        pix.push_back((int)iter[2]);
      }
    }
    assert(iter == buf.end());
  }

  return pix;
}

int main(int argc, const char *argv[]) {
  try {
    cxxopts::Options options("embed-image", "Embed 128x128 RGB texture images into C++ arrays");

    // clang-format off
    options.add_options()
      ("h,help", "Print this help message and exit")
      ("n,name", "The base name for the C++ variable", cxxopts::value<std::string>()->default_value("BLUE_NOISE_TEXTURE"))
      ("o,output", "The output path of the embedded C++", cxxopts::value<std::string>()->default_value("bluenoise.cpp"))
      ("images", "The PNG textures to embed", cxxopts::value<std::vector<std::string>>())
    ;

    options.positional_help("IMAGE...");
    options.parse_positional({"images"});
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.contains("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }
    auto images = result["images"].as<std::vector<std::string>>();

    std::ofstream out(result["output"].as<std::string>());
    out << "#include \"specula/macros.hpp\"\n";
    out << "#include <cstdint>\n\n";
    out << "namespace specula {\n";
    out << "SPECULA_CONST uint16_t " << result["name"].as<std::string>() << "[" << images.size() * 3
        << "]";

    bool first_image = true;
    unsigned int xres = 0, yres = 0;
    for (const auto &filename : images) {
      unsigned int xr = 0, yr = 0;
      std::vector<uint16_t> png = read_png(filename, &xr, &yr);
      if (xres == 0) {
        xres = xr;
        yres = yr;
      } else {
        assert(xres == xr);
        assert(yres == yr);
      }
      assert(png.size() == 3 * xres * yres);

      if (first_image) {
        first_image = false;
        out << "[" << yres << "][" << xres << "] = {\n";
      }

      for (unsigned int c = 0; c < 3; ++c) {
        int offset = c;
        out << "    {\n";
        for (unsigned int y = 0; y < yres; ++y) {
          out << "        { ";
          for (unsigned int x = 0; x < xres; ++x, offset += 3) {
            out << std::setw(5) << png[offset] << ", ";
          }
          out << " },\n";
        }
        out << "    },\n";
      }
    }
    out << "};\n";
    out << "}\n";

    out.close();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    exit(1);
  }
}
