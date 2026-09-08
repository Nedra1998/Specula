#include "specula/util/noise.hpp"

#include "specula/types.hpp"
#include "specula/util/math/functions.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"
#include "specula/util/vecmath/tuple3.hpp"

static constexpr size_t NOISE_PERM_SIZE = 256;

static SPECULA_CONST int NOISE_PERM[2 * NOISE_PERM_SIZE] = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103,
    30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,
    197, 62,  94,  252, 219, 203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174,
    20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231,
    83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143,
    54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169, 200, 196,
    135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124,
    123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,
    182, 189, 28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101,
    155, 167, 43,  172, 9,   129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185,
    112, 104, 218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,
    51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184, 84,  204, 176,
    115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114, 67,  29,  24,  72,  243,
    141, 128, 195, 78,  66,  215, 61,  156, 180, 151, 160, 137, 91,  90,  15,  131, 13,  201, 95,
    96,  53,  194, 233, 7,   225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,
    190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,  11,  32,
    57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165, 71,
    134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105,
    92,  41,  55,  46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209,
    76,  132, 187, 208, 89,  18,  169, 200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198,
    173, 186, 3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,
    212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213, 119, 248,
    152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,
    98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,  242, 193,
    238, 210, 144, 12,  191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214,
    31,  181, 199, 106, 157, 184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236,
    205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180};

namespace specula {
  SPECULA_CPU_GPU inline Float grad(int x, int y, int z, Float dx, Float dy, Float dz);
  SPECULA_CPU_GPU inline Float noise_weight(Float t);
} // namespace specula

specula::Float specula::noise(Float x, Float y, Float z) {
  x = pstd::fmod(x, Float(1 << 30));
  y = pstd::fmod(y, Float(1 << 30));
  z = pstd::fmod(z, Float(1 << 30));
  int ix = static_cast<int>(pstd::floor(x)), iy = static_cast<int>(pstd::floor(y)),
      iz = static_cast<int>(pstd::floor(z));
  Float dx = x - static_cast<Float>(ix), dy = y - static_cast<Float>(iy),
        dz = z - static_cast<Float>(iz);

  ix &= NOISE_PERM_SIZE - 1;
  iy &= NOISE_PERM_SIZE - 1;
  iz &= NOISE_PERM_SIZE - 1;

  Float w000 = grad(ix, iy, iz, dx, dy, dz);
  Float w100 = grad(ix + 1, iy, iz, dx - 1, dy, dz);
  Float w010 = grad(ix, iy + 1, iz, dx, dy - 1, dz);
  Float w001 = grad(ix, iy, iz + 1, dx, dy, dz - 1);
  Float w110 = grad(ix + 1, iy + 1, iz, dx - 1, dy - 1, dz);
  Float w101 = grad(ix + 1, iy, iz + 1, dx - 1, dy, dz - 1);
  Float w011 = grad(ix, iy + 1, iz + 1, dx, dy - 1, dz - 1);
  Float w111 = grad(ix + 1, iy + 1, iz + 1, dx - 1, dy - 1, dz - 1);

  Float wx = noise_weight(dx), wy = noise_weight(dy), wz = noise_weight(dz);
  Float x00 = lerp(wx, w000, w100);
  Float x10 = lerp(wx, w010, w110);
  Float x01 = lerp(wx, w001, w101);
  Float x11 = lerp(wx, w011, w111);
  Float y0 = lerp(wy, x00, x10);
  Float y1 = lerp(wy, x01, x11);
  return lerp(wz, y0, y1);
}

specula::Float specula::noise(Point3f p) { return noise(p.x, p.y, p.z); }

inline specula::Float specula::grad(int x, int y, int z, Float dx, Float dy, Float dz) {
  int h = NOISE_PERM[NOISE_PERM[NOISE_PERM[x] + y] + z];
  h &= 15;
  Float u = h < 8 || h == 12 || h == 13 ? dx : dy;
  Float v = h < 4 || h == 12 || h == 13 ? dy : dz;
  return (((h & 1) != 0) ? -u : u) + (((h & 2) != 0) ? -v : v);
}

inline specula::Float specula::noise_weight(Float t) {
  return 6 * pow<5>(t) - 15 * pow<4>(t) + 10 * pow<3>(t);
}

specula::Vector3f specula::dnoise(Point3f p) {
  Float delta = 0.01f;
  Float n = noise(p);
  Point3f noise_delta(noise(p + Vector3f(delta, 0, 0)), noise(p + Vector3f(0, delta, 0)),
                      noise(p + Vector3f(0, 0, delta)));
  return (noise_delta - Point3f(n, n, n)) / delta;
}

specula::Float specula::fbm(Point3f p, Vector3f dpdx, Vector3f dpdy, Float omega, int octaves) {
  Float len2 = std::max(length_squared(dpdx), length_squared(dpdy));
  Float n = clamp(-1 - log2(len2) / 2, 0, octaves);
  int nint = static_cast<int>(pstd::floor(n));

  Float sum = 0, lambda = 1, o = 1;
  for (int i = 0; i < nint; ++i) {
    sum += o * noise(lambda * p);
    lambda *= 1.99f;
    o *= omega;
  }
  Float npartial = n - static_cast<Float>(nint);
  sum += o * smooth_step(npartial, 0.3f, 0.7f) * noise(lambda * p);

  return sum;
}

specula::Float specula::turbulence(Point3f p, Vector3f dpdx, Vector3f dpdy, Float omega,
                                   int octaves) {
  Float len2 = std::max(length_squared(dpdx), length_squared(dpdy));
  Float n = clamp(-1 - log2(len2) / 2, 0, octaves);
  int nint = static_cast<int>(pstd::floor(n));

  Float sum = 0, lambda = 1, o = 1;
  for (int i = 0; i < nint; ++i) {
    sum += o * std::abs(noise(lambda * p));
    lambda *= 1.99f;
    o *= omega;
  }
  Float npartial = n - static_cast<Float>(nint);
  sum += o * lerp(smooth_step(npartial, 0.3f, 0.7f), 0.2, std::abs(noise(lambda * p)));
  for (int i = nint; i < octaves; ++i) {
    sum += o * 0.2f;
    o *= omega;
  }

  return sum;
}
