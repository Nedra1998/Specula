#ifndef SPECULA_UTIL_SOBOLMATRICES_HPP
#define SPECULA_UTIL_SOBOLMATRICES_HPP

#include <cstddef>
#include <cstdint>

#include "specula/macros.hpp"

namespace specula {
  static constexpr size_t N_SOBOL_DIMENSIONS = 1024;
  static constexpr size_t SOBOL_MATRIX_SIZE = 52;

  extern SPECULA_CONST uint32_t SOBOL_MATRICES_32[N_SOBOL_DIMENSIONS * SOBOL_MATRIX_SIZE];
  extern SPECULA_CONST uint64_t VDC_SOBOL_MATRICES[][SOBOL_MATRIX_SIZE];
  extern SPECULA_CONST uint64_t VDC_SOBOL_MATRICES_INV[][SOBOL_MATRIX_SIZE];
} // namespace specula

#endif // SPECULA_UTIL_SOBOLMATRICES_HPP
