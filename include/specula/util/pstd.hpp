/**
 * @file
 * @brief Umbrella header for `specula::pstd`
 *
 * Include this rather than the individual `pstd/` headers (they are IWYU-private).
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
 * @brief Device-safe subset of the C++ standard library, ported from pbrt-v4's `pstd`.
 *
 * Most of `std` cannot be called from GPU code, so pbrt reimplements the pieces it needs
 * (`array`, `optional`, `span`, `tuple`, `vector`, `complex`, `pmr` allocators, some `<cmath>`
 * and `<utility>`) with every member marked `SPECULA_CPU_GPU`. Interfaces follow `std` closely
 * but are often smaller; per-entity docs note where they differ.
 *
 * @see https://en.cppreference.com/w/cpp/standard_library
 */
namespace specula::pstd {}

#endif // SPECULA_UTIL_PSTD_HPP
