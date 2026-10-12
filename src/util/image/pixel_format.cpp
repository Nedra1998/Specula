#include "specula/util/image/pixel_format.hpp"

SPECULA_CPU_GPU int specula::texel_bytes(PixelFormat format) {
  switch (format) {
  case PixelFormat::U256:
    return 1;
  case PixelFormat::HALF:
    return 2;
  case PixelFormat::FLOAT:
    return 4;
  }
}
