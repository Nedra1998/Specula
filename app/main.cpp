#include <iostream>

#include <cxxopts.hpp>
#include <specula/version.hpp>

int main(int argc, char const *argv[]) {
  cxxopts::Options options("specula", "C++20 Physically Based Renderer");

  // clang-format off
  options.add_options()
    ("h,help", "Print this help message and exit")
    ("V,version", "Print the version information and exit")
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  } else if (result.count("version")) {
    std::cout << SPECULA_VERSION << std::endl;
    exit(0);
  }

  return 0;
}
