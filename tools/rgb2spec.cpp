#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#if defined(_WIN32)
#  include <io.h>
#  define IS_STDERR_TTY() (_isatty(_fileno(stderr)) != 0)
#else
#  include <unistd.h>
#  define IS_STDERR_TTY() (isatty(fileno(stderr)) != 0)
#endif

constexpr double CIE_LAMBDA_MIN = 360.0;
constexpr double CIE_LAMBDA_MAX = 830.0;
constexpr size_t CIE_SAMPLES = 95;
constexpr size_t CIE_FINE_SAMPLES = (CIE_SAMPLES - 1) * 3 + 1;
constexpr double CIE_LAMBDA_RANGE = CIE_LAMBDA_MAX - CIE_LAMBDA_MIN;
constexpr double RGB2SPEC_EPSILON = 1e-4;
constexpr double RGB2SPEC_COEFF_LIMIT = 200.0;

const double CIE_X[CIE_SAMPLES] = {
    0.000129900000, 0.000232100000, 0.000414900000, 0.000741600000, 0.001368000000, 0.002236000000,
    0.004243000000, 0.007650000000, 0.014310000000, 0.023190000000, 0.043510000000, 0.077630000000,
    0.134380000000, 0.214770000000, 0.283900000000, 0.328500000000, 0.348280000000, 0.348060000000,
    0.336200000000, 0.318700000000, 0.290800000000, 0.251100000000, 0.195360000000, 0.142100000000,
    0.095640000000, 0.057950010000, 0.032010000000, 0.014700000000, 0.004900000000, 0.002400000000,
    0.009300000000, 0.029100000000, 0.063270000000, 0.109600000000, 0.165500000000, 0.225749900000,
    0.290400000000, 0.359700000000, 0.433449900000, 0.512050100000, 0.594500000000, 0.678400000000,
    0.762100000000, 0.842500000000, 0.916300000000, 0.978600000000, 1.026300000000, 1.056700000000,
    1.062200000000, 1.045600000000, 1.002600000000, 0.938400000000, 0.854449900000, 0.751400000000,
    0.642400000000, 0.541900000000, 0.447900000000, 0.360800000000, 0.283500000000, 0.218700000000,
    0.164900000000, 0.121200000000, 0.087400000000, 0.063600000000, 0.046770000000, 0.032900000000,
    0.022700000000, 0.015840000000, 0.011359160000, 0.008110916000, 0.005790346000, 0.004109457000,
    0.002899327000, 0.002049190000, 0.001439971000, 0.000999949300, 0.000690078600, 0.000476021300,
    0.000332301100, 0.000234826100, 0.000166150500, 0.000117413000, 0.000083075270, 0.000058706520,
    0.000041509940, 0.000029353260, 0.000020673830, 0.000014559770, 0.000010253980, 0.000007221456,
    0.000005085868, 0.000003581652, 0.000002522525, 0.000001776509, 0.000001251141};

const double CIE_Y[CIE_SAMPLES] = {
    0.000003917000, 0.000006965000, 0.000012390000, 0.000022020000, 0.000039000000, 0.000064000000,
    0.000120000000, 0.000217000000, 0.000396000000, 0.000640000000, 0.001210000000, 0.002180000000,
    0.004000000000, 0.007300000000, 0.011600000000, 0.016840000000, 0.023000000000, 0.029800000000,
    0.038000000000, 0.048000000000, 0.060000000000, 0.073900000000, 0.090980000000, 0.112600000000,
    0.139020000000, 0.169300000000, 0.208020000000, 0.258600000000, 0.323000000000, 0.407300000000,
    0.503000000000, 0.608200000000, 0.710000000000, 0.793200000000, 0.862000000000, 0.914850100000,
    0.954000000000, 0.980300000000, 0.994950100000, 1.000000000000, 0.995000000000, 0.978600000000,
    0.952000000000, 0.915400000000, 0.870000000000, 0.816300000000, 0.757000000000, 0.694900000000,
    0.631000000000, 0.566800000000, 0.503000000000, 0.441200000000, 0.381000000000, 0.321000000000,
    0.265000000000, 0.217000000000, 0.175000000000, 0.138200000000, 0.107000000000, 0.081600000000,
    0.061000000000, 0.044580000000, 0.032000000000, 0.023200000000, 0.017000000000, 0.011920000000,
    0.008210000000, 0.005723000000, 0.004102000000, 0.002929000000, 0.002091000000, 0.001484000000,
    0.001047000000, 0.000740000000, 0.000520000000, 0.000361100000, 0.000249200000, 0.000171900000,
    0.000120000000, 0.000084800000, 0.000060000000, 0.000042400000, 0.000030000000, 0.000021200000,
    0.000014990000, 0.000010600000, 0.000007465700, 0.000005257800, 0.000003702900, 0.000002607800,
    0.000001836600, 0.000001293400, 0.000000910930, 0.000000641530, 0.000000451810};

const double CIE_Z[CIE_SAMPLES] = {
    0.000606100000, 0.001086000000, 0.001946000000, 0.003486000000, 0.006450001000, 0.010549990000,
    0.020050010000, 0.036210000000, 0.067850010000, 0.110200000000, 0.207400000000, 0.371300000000,
    0.645600000000, 1.039050100000, 1.385600000000, 1.622960000000, 1.747060000000, 1.782600000000,
    1.772110000000, 1.744100000000, 1.669200000000, 1.528100000000, 1.287640000000, 1.041900000000,
    0.812950100000, 0.616200000000, 0.465180000000, 0.353300000000, 0.272000000000, 0.212300000000,
    0.158200000000, 0.111700000000, 0.078249990000, 0.057250010000, 0.042160000000, 0.029840000000,
    0.020300000000, 0.013400000000, 0.008749999000, 0.005749999000, 0.003900000000, 0.002749999000,
    0.002100000000, 0.001800000000, 0.001650001000, 0.001400000000, 0.001100000000, 0.001000000000,
    0.000800000000, 0.000600000000, 0.000340000000, 0.000240000000, 0.000190000000, 0.000100000000,
    0.000049999990, 0.000030000000, 0.000020000000, 0.000010000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000};

#define N(x) (x / 10566.864005283874576)
const double CIE_D65[CIE_SAMPLES] = {
    N(46.6383), N(49.3637), N(52.0891), N(51.0323), N(49.9755), N(52.3118), N(54.6482), N(68.7015),
    N(82.7549), N(87.1204), N(91.486),  N(92.4589), N(93.4318), N(90.057),  N(86.6823), N(95.7736),
    N(104.865), N(110.936), N(117.008), N(117.41),  N(117.812), N(116.336), N(114.861), N(115.392),
    N(115.923), N(112.367), N(108.811), N(109.082), N(109.354), N(108.578), N(107.802), N(106.296),
    N(104.79),  N(106.239), N(107.689), N(106.047), N(104.405), N(104.225), N(104.046), N(102.023),
    N(100.0),   N(98.1671), N(96.3342), N(96.0611), N(95.788),  N(92.2368), N(88.6856), N(89.3459),
    N(90.0062), N(89.8026), N(89.5991), N(88.6489), N(87.6987), N(85.4936), N(83.2886), N(83.4939),
    N(83.6992), N(81.863),  N(80.0268), N(80.1207), N(80.2146), N(81.2462), N(82.2778), N(80.281),
    N(78.2842), N(74.0027), N(69.7213), N(70.6652), N(71.6091), N(72.979),  N(74.349),  N(67.9765),
    N(61.604),  N(65.7448), N(69.8856), N(72.4863), N(75.087),  N(69.3398), N(63.5927), N(55.0054),
    N(46.4182), N(56.6118), N(66.8054), N(65.0941), N(63.3828), N(63.8434), N(64.304),  N(61.8779),
    N(59.4519), N(55.7054), N(51.959),  N(54.6998), N(57.4406), N(58.8765), N(60.3125)};
#undef N

#define N(x) (x / 106.8)
const double CIE_E[CIE_SAMPLES] = {
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0)};
#undef N

#define N(x) (x / 10503.2)
const double CIE_D50[CIE_SAMPLES] = {
    N(23.942000),  N(25.451000),  N(26.961000),  N(25.724000),  N(24.488000),  N(27.179000),
    N(29.871000),  N(39.589000),  N(49.308000),  N(52.910000),  N(56.513000),  N(58.273000),
    N(60.034000),  N(58.926000),  N(57.818000),  N(66.321000),  N(74.825000),  N(81.036000),
    N(87.247000),  N(88.930000),  N(90.612000),  N(90.990000),  N(91.368000),  N(93.238000),
    N(95.109000),  N(93.536000),  N(91.963000),  N(93.843000),  N(95.724000),  N(96.169000),
    N(96.613000),  N(96.871000),  N(97.129000),  N(99.614000),  N(102.099000), N(101.427000),
    N(100.755000), N(101.536000), N(102.317000), N(101.159000), N(100.000000), N(98.868000),
    N(97.735000),  N(98.327000),  N(98.918000),  N(96.208000),  N(93.499000),  N(95.593000),
    N(97.688000),  N(98.478000),  N(99.269000),  N(99.155000),  N(99.042000),  N(97.382000),
    N(95.722000),  N(97.290000),  N(98.857000),  N(97.262000),  N(95.667000),  N(96.929000),
    N(98.190000),  N(100.597000), N(103.003000), N(101.068000), N(99.133000),  N(93.257000),
    N(87.381000),  N(89.492000),  N(91.604000),  N(92.246000),  N(92.889000),  N(84.872000),
    N(76.854000),  N(81.683000),  N(86.511000),  N(89.546000),  N(92.580000),  N(85.405000),
    N(78.230000),  N(67.961000),  N(57.692000),  N(70.307000),  N(82.923000),  N(80.599000),
    N(78.274000),  N(0),          N(0),          N(0),          N(0),          N(0),
    N(0),          N(0),          N(0),          N(0)};
#undef N

#define N(x) (x / 10536.3)
const double CIE_D60[CIE_SAMPLES] = {
    N(38.683115),  N(41.014457),  N(42.717548),  N(42.264182),  N(41.454941),  N(41.763698),
    N(46.605319),  N(59.226938),  N(72.278594),  N(78.231500),  N(80.440600),  N(82.739580),
    N(82.915027),  N(79.009168),  N(77.676264),  N(85.163609),  N(95.681274),  N(103.267764),
    N(107.954821), N(109.777964), N(109.559187), N(108.418402), N(107.758141), N(109.071548),
    N(109.671404), N(106.734741), N(103.707873), N(103.981942), N(105.232199), N(105.235867),
    N(104.427667), N(103.052881), N(102.522934), N(104.371416), N(106.052671), N(104.948900),
    N(103.315154), N(103.416286), N(103.538599), N(102.099304), N(100.000000), N(97.992725),
    N(96.751421),  N(97.102402),  N(96.712823),  N(93.174457),  N(89.921479),  N(90.351933),
    N(91.999793),  N(92.384009),  N(92.098710),  N(91.722859),  N(90.646003),  N(88.327552),
    N(86.526483),  N(87.034239),  N(87.579186),  N(85.884584),  N(83.976140),  N(83.743140),
    N(84.724074),  N(86.450818),  N(87.493491),  N(86.546330),  N(83.483070),  N(78.268785),
    N(74.172451),  N(74.275184),  N(76.620385),  N(79.423856),  N(79.051849),  N(71.763360),
    N(65.471371),  N(67.984085),  N(74.106079),  N(78.556612),  N(79.527120),  N(75.584935),
    N(67.307163),  N(55.275106),  N(49.273538),  N(59.008629),  N(70.892412),  N(70.950115),
    N(67.163996),  N(67.445480),  N(68.171371),  N(66.466636),  N(62.989809),  N(58.067786),
    N(54.990892),  N(56.915942),  N(60.825601),  N(62.987850)};
#undef N

const double XYZ_TO_SRGB[3][3] = {{3.240479, -1.537150, -0.498535},
                                  {-0.969256, 1.875991, 0.041556},
                                  {0.055648, -0.204043, 1.057311}};
const double SRGB_TO_XYZ[3][3] = {
    {0.412453, 0.357580, 0.180423}, {0.212671, 0.715160, 0.072169}, {0.019334, 0.119193, 0.950227}};

const double XYZ_TO_XYZ[3][3] = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
};

const double XYZ_TO_ERGB[3][3] = {
    {2.689989, -1.276020, -0.413844},
    {-1.022095, 1.978261, 0.043821},
    {0.061203, -0.224411, 1.162859},
};
const double ERGB_TO_XYZ[3][3] = {
    {0.496859, 0.339094, 0.164047},
    {0.256193, 0.678188, 0.065619},
    {0.023290, 0.113031, 0.863978},
};

const double XYZ_TO_PROPHOTO_RGB[3][3] = {{1.3459433, -0.2556075, -0.0511118},
                                          {-0.5445989, 1.5081673, 0.0205351},
                                          {0.0000000, 0.0000000, 1.2118128}};
const double PROPHOTO_RGB_TO_XYZ[3][3] = {{0.7976749, 0.1351917, 0.0313534},
                                          {0.2880402, 0.7118741, 0.0000857},
                                          {0.0000000, 0.0000000, 0.8252100}};

const double XYZ_TO_ACES2065_1[3][3] = {{1.0498110175, 0.0000000000, -0.0000974845},
                                        {-0.4959030231, 1.3733130458, 0.0982400361},
                                        {0.0000000000, 0.0000000000, 0.9912520182}};
const double ACES2065_1_TO_XYZ[3][3] = {{0.9525523959, 0.0000000000, 0.0000936786},
                                        {0.3439664498, 0.7281660966, -0.0721325464},
                                        {0.0000000000, 0.0000000000, 1.0088251844}};

const double XYZ_TO_REC2020[3][3] = {{1.7166511880, -0.3556707838, -0.2533662814},
                                     {-0.6666843518, 1.6164812366, 0.0157685458},
                                     {0.0176398574, -0.0427706133, 0.9421031212}};
const double REC2020_TO_XYZ[3][3] = {{0.6369580483, 0.1446169036, 0.1688809752},
                                     {0.2627002120, 0.6779980715, 0.0593017165},
                                     {0.0000000000, 0.0280726930, 1.0609850577}};

const double XYZ_TO_DCIP3[3][3] = {{2.4931748, -0.93126315, -0.40265882},
                                   {-0.82950425, 1.7626965, 0.023625137},
                                   {0.035853732, -0.07618918, 0.9570952}};
const double DCIP3_TO_XYZ[3][3] = {{0.48663378, 0.26566276, 0.19817366},
                                   {0.22900413, 0.69172573, 0.079269454},
                                   {0., 0.04511256, 1.0437145}};

enum class Gamut : uint8_t {
  ACES2065_1,
  DCI_P3,
  ERGB,
  ProPhotoRGB,
  REC2020,
  SRGB,
  XYZ,
};

double lambda_tbl[CIE_FINE_SAMPLES];
double rgb_tbl[3][CIE_FINE_SAMPLES];
double rgb_to_xyz[3][3], xyz_to_rgb[3][3];
double xyz_whitepoint[3];

double sigmoid(double x) { return 0.5 * x / std::sqrt(1.0 + x * x) + 0.5; }

double smoothstep(double x) { return x * x * (3.0 - 2.0 * x); }

constexpr double sqr(double x) { return x * x; }

double cie_interp(const double *data, double x) {
  x -= CIE_LAMBDA_MIN;
  x *= (CIE_SAMPLES - 1) / CIE_LAMBDA_RANGE;
  auto offset = static_cast<int>(x);
  offset = std::clamp(offset, 0, static_cast<int>(CIE_SAMPLES) - 2);
  double weight = x - offset;
  return (1.0 - weight) * data[offset] + weight * data[offset + 1];
}

void cie_lab(double *p) {
  double x = 0.0, y = 0.0, z = 0.0, xw = xyz_whitepoint[0], yw = xyz_whitepoint[1],
         zw = xyz_whitepoint[2];

  for (int j = 0; j < 3; ++j) {
    x += p[j] * rgb_to_xyz[0][j];
    y += p[j] * rgb_to_xyz[1][j];
    z += p[j] * rgb_to_xyz[2][j];
  }

  auto f = [](double t) -> double {
    constexpr double delta = 6.0 / 29.0;
    if (t > delta * delta * delta) {
      return cbrt(t);
    } else {
      return t / (delta * delta * 3.0) + (4.0 / 29.0);
    }
  };

  p[0] = 116.0 * f(y / yw) - 16.0;
  p[1] = 500.0 * (f(x / xw) - f(y / yw));
  p[2] = 200.0 * (f(y / yw) - f(z / zw));
}

template <typename Func> void parallel_for(unsigned int start, unsigned int end, Func &&func) {
  if (start >= end) {
    return;
  }
  unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
  unsigned int total_elements = end - start;
  num_threads = std::min(num_threads, total_elements);

  unsigned int chunk_size = total_elements / num_threads;
  unsigned int remainder = total_elements % num_threads;

  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  unsigned int current_start = start;

  for (unsigned int i = 0; i < num_threads; ++i) {
    unsigned int current_end = current_start + chunk_size + (i < remainder ? 1 : 0);

    threads.emplace_back([current_start, current_end, &func]() {
      for (unsigned int j = current_start; j < current_end; ++j) {
        func(j);
      }
    });

    current_start = current_end;
  }

  for (auto &t : threads) {
    t.join();
  }
}

void initialize_tables(Gamut gamut) {
  // Set the output tables to 0.0
  std::fill(&rgb_tbl[0][0], &rgb_tbl[0][0] + 3 * CIE_FINE_SAMPLES, 0.0);
  std::ranges::fill(xyz_whitepoint, 0.0);

  const double *illuminant = nullptr;
  switch (gamut) {
  case Gamut::ACES2065_1:
    illuminant = CIE_D60;
    std::copy(&XYZ_TO_ACES2065_1[0][0], &XYZ_TO_ACES2065_1[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&ACES2065_1_TO_XYZ[0][0], &ACES2065_1_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  case Gamut::DCI_P3:
    illuminant = CIE_D65;
    std::copy(&XYZ_TO_DCIP3[0][0], &XYZ_TO_DCIP3[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&DCIP3_TO_XYZ[0][0], &DCIP3_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  case Gamut::ERGB:
    illuminant = CIE_E;
    std::copy(&XYZ_TO_ERGB[0][0], &XYZ_TO_ERGB[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&ERGB_TO_XYZ[0][0], &ERGB_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  case Gamut::ProPhotoRGB:
    illuminant = CIE_D50;
    std::copy(&XYZ_TO_PROPHOTO_RGB[0][0], &XYZ_TO_PROPHOTO_RGB[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&PROPHOTO_RGB_TO_XYZ[0][0], &PROPHOTO_RGB_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  case Gamut::REC2020:
    illuminant = CIE_D65;
    std::copy(&XYZ_TO_REC2020[0][0], &XYZ_TO_REC2020[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&REC2020_TO_XYZ[0][0], &REC2020_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  case Gamut::SRGB:
    illuminant = CIE_D65;
    std::copy(&XYZ_TO_SRGB[0][0], &XYZ_TO_SRGB[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&SRGB_TO_XYZ[0][0], &SRGB_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  case Gamut::XYZ:
    illuminant = CIE_E;
    std::copy(&XYZ_TO_XYZ[0][0], &XYZ_TO_XYZ[0][0] + 9, &xyz_to_rgb[0][0]);
    std::copy(&XYZ_TO_XYZ[0][0], &XYZ_TO_XYZ[0][0] + 9, &rgb_to_xyz[0][0]);
    break;
  }

  double h = CIE_LAMBDA_RANGE / (CIE_FINE_SAMPLES - 1);
  for (size_t i = 0; i < CIE_FINE_SAMPLES; ++i) {
    double lambda = CIE_LAMBDA_MIN + (static_cast<double>(i) * h);
    double xyz[3] = {cie_interp(CIE_X, lambda), cie_interp(CIE_Y, lambda),
                     cie_interp(CIE_Z, lambda)};
    double illum = cie_interp(illuminant, lambda);

    double weight = 3.0 / 8.0 * h;
    if (i == 0 || i == CIE_FINE_SAMPLES - 1) {
    } else if ((i - 1) % 3 == 2) {
      weight *= 2.0f;
    } else {
      weight *= 3.0f;
    }

    lambda_tbl[i] = lambda;
    for (int k = 0; k < 3; ++k) {
      for (int j = 0; j < 3; ++j) {
        rgb_tbl[k][i] += xyz_to_rgb[k][j] * xyz[j] * illum * weight;
      }
    }

    for (int j = 0; j < 3; ++j) {
      xyz_whitepoint[j] += xyz[j] * illum * weight;
    }
  }
}

int lup_decompose(double **A, int N, double Tol, int *P) {
  for (int i = 0; i <= N; i++) {
    P[i] = i;
  }

  for (int i = 0; i < N; i++) {
    double maxA = 0.0;
    int imax = i;

    for (int k = i; k < N; k++) {
      double absA = fabs(A[k][i]);
      if (absA > maxA) {
        maxA = absA;
        imax = k;
      }
    }

    if (maxA < Tol) {
      return 0;
    }

    if (imax != i) {
      std::swap(P[i], P[imax]);
      std::swap(A[i], A[imax]);
      P[N]++;
    }

    for (int j = i + 1; j < N; j++) {
      A[j][i] /= A[i][i];

      for (int k = i + 1; k < N; k++) {
        A[j][k] -= A[j][i] * A[i][k];
      }
    }
  }

  return 1;
}

void lup_solve(double **const A, const int *P, const double *b, int N, double *x) {
  for (int i = 0; i < N; i++) {
    x[i] = b[P[i]];

    for (int k = 0; k < i; k++) {
      x[i] -= A[i][k] * x[k];
    }
  }

  for (int i = N - 1; i >= 0; i--) {
    for (int k = i + 1; k < N; k++) {
      x[i] -= A[i][k] * x[k];
    }

    x[i] = x[i] / A[i][i];
  }
}

void eval_residual(const double *coeffs, const double *rgb, double *residual) {
  double out[3] = {0.0, 0.0, 0.0};

  for (int i = 0; i < CIE_FINE_SAMPLES; ++i) {
    double lambda = (lambda_tbl[i] - CIE_LAMBDA_MIN) / CIE_LAMBDA_RANGE;
    double x = 0.0;
    for (int i = 0; i < 3; ++i) {
      x = x * lambda + coeffs[i];
    }

    double s = sigmoid(x);
    for (int j = 0; j < 3; ++j) {
      out[j] += rgb_tbl[j][i] * s;
    }
  }

  cie_lab(out);
  std::copy(rgb, rgb + 3, residual);
  cie_lab(residual);

  for (int j = 0; j < 3; ++j) {
    residual[j] -= out[j];
  }
}

void eval_jacobian(const double *coeffs, const double *rgb, double **jac) {
  double r0[3], r1[3], tmp[3];

  for (int i = 0; i < 3; ++i) {
    std::copy(coeffs, coeffs + 3, tmp);
    tmp[i] -= RGB2SPEC_EPSILON;
    eval_residual(tmp, rgb, r0);

    std::copy(coeffs, coeffs + 3, tmp);
    tmp[i] += RGB2SPEC_EPSILON;
    eval_residual(tmp, rgb, r1);

    for (int j = 0; j < 3; ++j) {
      jac[j][i] = (r1[j] - r0[j]) * 1.0 / (2 * RGB2SPEC_EPSILON);
    }
  }
}

void gauss_newton(const double rgb[3], double coeffs[3], int it = 15) {
  double r = 0;
  for (int i = 0; i < it; ++i) {
    double j0[3], j1[3], j2[3], *j[3] = {j0, j1, j2};
    double residual[3];

    eval_residual(coeffs, rgb, residual);
    eval_jacobian(coeffs, rgb, j);

    int p[4];
    int rv = lup_decompose(j, 3, 1e-15, p);
    if (rv != 1) {
      spdlog::critical("LU decomposition failed: RGB ({}, {}, {}) -> ({}, {}, {})", rgb[0], rgb[1],
                       rgb[2], coeffs[0], coeffs[1], coeffs[2]);
      exit(-1);
    }

    double x[3];
    lup_solve(j, p, residual, 3, x);
    r = 0.0;
    for (int j = 0; j < 3; ++j) {
      coeffs[j] -= x[j];
      r += residual[j] * residual[j];
    }
    double max = std::max({coeffs[0], coeffs[1], coeffs[2]});
    if (max > RGB2SPEC_COEFF_LIMIT) {
      for (int j = 0; j < 3; ++j) {
        coeffs[j] *= RGB2SPEC_COEFF_LIMIT / max;
      }
    }

    if (r < 1e-6) {
      break;
    }
  }
}

int main(int argc, const char *argv[]) {
  try {
    cxxopts::Options options("rgb2spec", "Generate RGB to Spectrum converstion tables");

    // clang-format off
    options.add_options("General")
      ("h,help", "Print this help message and exit")
      ("v,verbose", "Enable debug/verbose logging", cxxopts::value<bool>())
      ("q,quiet", "Suppress non-error logging", cxxopts::value<bool>())
    ;

    options.add_options()
      ("resolution", "The resolution of the generated tables", cxxopts::value<unsigned>())
      ("output", "The output path of the generated C++", cxxopts::value<std::string>())
      ("g,gamut", "The color gamut to generate a table for", cxxopts::value<std::string>()->default_value("sRGB"))
    ;

    options.positional_help("RESOLUTION OUTPUT");
    options.parse_positional({"resolution", "output"});
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

    // Validate arguments
    auto gamut_opt = magic_enum::enum_cast<Gamut>(result["gamut"].as<std::string>(),
                                                  magic_enum::case_insensitive);
    if (!gamut_opt.has_value()) {
      spdlog::error("Invalid gamut choice '{}'. Valid options are: {}",
                    result["gamut"].as<std::string>(), magic_enum::enum_names<Gamut>());
      return 1;
    }
    Gamut gamut = gamut_opt.value();

    const unsigned resolution = result["resolution"].as<unsigned>();
    if (resolution < 2) {
      spdlog::error("Invalid resolution '{}'. Resolution must be at least 2",
                    result["resolution"].as<unsigned>());
      return 1;
    }

    const std::string output = result["output"].as<std::string>();

    spdlog::info("Generating {} RGB-to-spectrum table (resolution {}, output: '{}')",
                 magic_enum::enum_name(gamut), resolution, output);
    initialize_tables(gamut);
    spdlog::debug("Color matching table sinitialized for {}", magic_enum::enum_name(gamut));
    spdlog::info("Optimizing {} spectra...", magic_enum::enum_name(gamut));

    std::vector<float> scale(resolution);
    for (int k = 0; k < resolution; ++k) {
      scale[k] = static_cast<float>(smoothstep(smoothstep(k / double(resolution - 1))));
    }

    const auto bufsize = size_t{3} * 3 * resolution * resolution * resolution;
    std::vector<float> out(bufsize);

    for (int l = 0; l < 3; ++l) {
      parallel_for(0, resolution, [&](unsigned int j) {
        const double y = j / double(resolution - 1);
        for (unsigned int i = 0; i < resolution; ++i) {
          const double x = i / double(resolution - 1);
          double coeffs[3], rgb[3];
          std::ranges::fill(coeffs, 0.0);

          unsigned int start = resolution / 5;
          for (unsigned int k = start; k < resolution; ++k) {
            const double b = scale[k];
            rgb[l] = b;
            rgb[(l + 1) % 3] = x * b;
            rgb[(l + 2) % 3] = y * b;

            gauss_newton(rgb, coeffs);

            constexpr double c0 = CIE_LAMBDA_MIN;
            constexpr double c1 = 1.0 / CIE_LAMBDA_RANGE;
            double ca = coeffs[0], cb = coeffs[1], cc = coeffs[2];

            unsigned int idx = ((l * resolution + k) * resolution + j) * resolution + i;

            out[3 * idx + 0] = float(ca * sqr(c1));
            out[3 * idx + 1] = float(cb * c1 - 2 * ca * c0 * sqr(c1));
            out[3 * idx + 2] = float(cc - cb * c0 * c1 + ca * sqr(c0 * c1));
          }

          std::ranges::fill(coeffs, 0.0);
          for (int k = (int)start; k >= 0; --k) {
            auto b = (double)scale[k];
            rgb[l] = b;
            rgb[(l + 1) % 3] = x * b;
            rgb[(l + 2) % 3] = y * b;

            gauss_newton(rgb, coeffs);

            constexpr double c0 = CIE_LAMBDA_MIN;
            constexpr double c1 = 1.0 / CIE_LAMBDA_RANGE;
            double ca = coeffs[0], cb = coeffs[1], cc = coeffs[2];

            unsigned int idx = ((l * resolution + k) * resolution + j) * resolution + i;

            out[3 * idx + 0] = float(ca * sqr(c1));
            out[3 * idx + 1] = float(cb * c1 - 2 * ca * c0 * sqr(c1));
            out[3 * idx + 2] = float(cc - cb * c0 * c1 + ca * sqr(c0 * c1));
          }
        }
      });
    }

    // Open the output file for writing
    std::ofstream fout(output);
    if (!fout.is_open()) {
      spdlog::error("Could not create output file: '{}'", output);
      return 1;
    }

    // Write the standard header
    fmt::println(fout, "namespace specula {{");
    fmt::println(fout, "extern const int {}_TO_SPECTRUM_TABLE_RES = {};",
                 magic_enum::enum_name(gamut), resolution);
    fmt::println(fout, "extern const float {}_TO_SPECTRUM_TABLE_SCALE[{}] = {{\n{:.9g}\n}};",
                 magic_enum::enum_name(gamut), resolution, fmt::join(scale, ", "));
    fmt::println(fout,
                 "extern const float {}_TO_SPECTRUM_TABLE_DATA[3][{res}][{res}][{res}][3] = {{",
                 magic_enum::enum_name(gamut), fmt::arg("res", resolution));

    auto iter = out.begin();
    for (int maxc = 0; maxc < 3; ++maxc) {
      fmt::println(fout, "    {{");
      for (int z = 0; z < resolution; ++z) {
        fmt::println(fout, "      {{");
        for (int y = 0; y < resolution; ++y) {
          fmt::print(fout, "        {{ ");
          for (int x = 0; x < resolution; ++x) {
            fmt::print(fout, "{{ ");
            for (int c = 0; c < 3; ++c) {
              fmt::print(fout, "{:15.9g}, ", *iter++);
            }
            fmt::print(fout, "}}, ");
          }
          fmt::println(fout, "}},");
        }
        fmt::println(fout, "      }},");
      }
      fmt::println(fout, "    }},");
    }

    fmt::print(fout, "}};\n}}");
    fout.close();
    if (fout.fail()) {
      spdlog::error("Failed while finalizing output file '{}'", output);
      return 1;
    }

    spdlog::info("Generated '{}' successfully", output);

  } catch (const std::exception &e) {
    fmt::print(stderr, "CLI Parsing Error: {}\n", e.what());
    exit(1);
  }
}
