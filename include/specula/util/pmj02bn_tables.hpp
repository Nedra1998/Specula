#ifndef SPECULA_UTIL_PMJ02BN_TABLES_HPP
#define SPECULA_UTIL_PMJ02BN_TABLES_HPP

#include <cstddef>
#include <cstdint>

#include "specula/macros.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  constexpr size_t N_PMJ02BN_SETS = 5;
  constexpr size_t N_PMJ02BN_SAMPLES = 65536;

  // TODO: try to move the definition into a compile-time generated source file
  // to reduce the size of the C++ source code
  extern SPECULA_CONST uint32_t PMJ02BN_SAMPLES[N_PMJ02BN_SETS][N_PMJ02BN_SAMPLES][2];

  SPECULA_CPU_GPU inline Point2f get_pmj02bn_sample(size_t set_index, size_t sample_index) {
    set_index %= N_PMJ02BN_SETS;
    DASSERT_LE(sample_index, N_PMJ02BN_SAMPLES);
    sample_index %= N_PMJ02BN_SAMPLES;

#ifdef SPECULA_IS_GPU_CODE
    return {static_cast<Float>(PMJ02BN_SAMPLES[set_index][sample_index][0] * 0x1p-32f),
            static_cast<Float>(PMJ02BN_SAMPLES[set_index][sample_index][1] * 0x1p-32f)};
#else
    return {static_cast<Float>(PMJ02BN_SAMPLES[set_index][sample_index][0] * 0x1p-32),
            static_cast<Float>(PMJ02BN_SAMPLES[set_index][sample_index][1] * 0x1p-32)};
#endif
  }
} // namespace specula

#endif // SPECULA_UTIL_PMJ02BN_TABLES_HPP
