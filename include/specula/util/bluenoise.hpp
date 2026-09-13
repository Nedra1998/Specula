#ifndef SPECULA_UTIL_BLUENOISE_HPP
#define SPECULA_UTIL_BLUENOISE_HPP

#include <cstddef>
#include <cstdint>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  static constexpr size_t BLUE_NOISE_RESOLUTION = 128;
  static constexpr size_t NUM_BLUE_NOISE_TEXTURES = 48;

  extern SPECULA_CONST uint16_t
      BLUE_NOISE_TEXTURE[NUM_BLUE_NOISE_TEXTURES][BLUE_NOISE_RESOLUTION][BLUE_NOISE_RESOLUTION];

  SPECULA_CPU_GPU inline float blue_noise(size_t texture_index, Point2i p) {
    ASSERT(texture_index >= 0 && p.x >= 0 && p.y >= 0);
    texture_index %= NUM_BLUE_NOISE_TEXTURES;
    size_t x = p.x % BLUE_NOISE_RESOLUTION, y = p.y % BLUE_NOISE_RESOLUTION;
    return static_cast<float>(BLUE_NOISE_TEXTURE[texture_index][x][y]) / 65535.0f;
  }
} // namespace specula

#endif // SPECULA_UTIL_BLUENOISE_HPP
