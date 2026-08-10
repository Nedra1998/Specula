/**
 * @file
 * @brief Portable Standard Library Export
 *
 * This file defines the specula::pstd namespace which is a
 * re-implementation of some of the standard library needed in order to allow
 * for a portable interface that is used for both CPU and GPU platforms.
 *
 * The actual implementation is split up within the `pstd` directory, but this
 * file acts as an export header for the reset of the library to use instead of
 * the individual header files.
 */

#ifndef SPECULA_UTIL_PSTD_HPP
#define SPECULA_UTIL_PSTD_HPP

// IWYU pragma: begin_exports
#include "specula/util/pstd/array.hpp"
#include "specula/util/pstd/complex.hpp"
#include "specula/util/pstd/math.hpp"
#include "specula/util/pstd/optional.hpp"
#include "specula/util/pstd/pmr.hpp"
#include "specula/util/pstd/span.hpp"
#include "specula/util/pstd/tuple.hpp"
#include "specula/util/pstd/utility.hpp"
#include "specula/util/pstd/vector.hpp"
// IWYU pragma: end_exports

/**
 * @brief Portable Standard Library available on both the host (CPU) and device (GPU)
 *
 * This namespace defines a subset of the C++ standard library to provide a
 * consistent interface that can be used on both the CPU and the GPU. The
 * functionality is the same as the corresponding entity in `std`.
 *
 * @see https://en.cppreference.com/
 */
namespace specula::pstd {}

#endif // SPECULA_UTIL_PSTD_HPP
