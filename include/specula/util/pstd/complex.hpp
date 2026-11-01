/**
 * @file
 * @brief Minimal GPU-safe complex number type (`pstd::complex`)
 *
 * `std::complex` is not usable in device code, so pbrt ships a small replacement that covers only
 * what the renderer needs: basic arithmetic, `norm`/`abs` and `sqrt` (used by the complex Fresnel
 * equations for conductors). Anything else from `<complex>` (`conj`, `exp`, `arg`, comparison,
 * compound assignment, ...) is intentionally absent.
 *
 * @see https://en.cppreference.com/w/cpp/numeric/complex
 */

#ifndef SPECULA_UTIL_PSTD_COMPLEX_HPP
#define SPECULA_UTIL_PSTD_COMPLEX_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include "specula/macros.hpp"
#include "specula/util/pstd/math.hpp"

namespace specula::pstd {
  /**
   * @brief Complex number @f$ re + i\,im @f$ with public `re`/`im` members.
   *
   * Unlike `std::complex` there is no default constructor, and the single-argument constructor is
   * implicit so that real scalars convert to complex values (this is what makes the mixed
   * `T op complex` friends and `return 0;` work).
   *
   * @tparam T Scalar type. `abs`/`sqrt` only work for `float` and `double`, since `pstd::math`
   * only provides those overloads.
   * @see https://en.cppreference.com/w/cpp/numeric/complex
   */
  template <typename T> struct complex {
    /** @brief Real value @f$ re + 0i @f$ (implicit). */
    SPECULA_CPU_GPU complex(T re) : re(re), im(0) {}
    /** @brief @f$ re + i\,im @f$. */
    SPECULA_CPU_GPU complex(T re, T im) : re(re), im(im) {}

    /** @brief Negation. */
    SPECULA_CPU_GPU complex operator-() const { return {-re, -im}; }

    /** @brief Addition. */
    SPECULA_CPU_GPU complex operator+(complex z) const { return {re + z.re, im + z.im}; }
    /** @brief Subtraction. */
    SPECULA_CPU_GPU complex operator-(complex z) const { return {re - z.re, im - z.im}; }
    /** @brief Multiplication @f$ (ac - bd) + i(ad + bc) @f$. */
    SPECULA_CPU_GPU complex operator*(complex z) const {
      return {re * z.re - im * z.im, re * z.im + im * z.re};
    }
    /**
     * @brief Division by the textbook formula
     * @f$ \frac{a + ib}{c + id} = \frac{(ac + bd) + i(bc - ad)}{c^2 + d^2} @f$.
     *
     * @note No scaling is done (no Smith's algorithm), so @f$ c^2 + d^2 @f$ can overflow or
     * underflow for large/small divisors; dividing by zero yields inf/NaN.
     * @todo The `1.0` literal makes the reciprocal a `double` computation even when `T = float`.
     */
    SPECULA_CPU_GPU complex operator/(complex z) const {
      T scale = 1.0 / (z.re * z.re + z.im * z.im);
      return {scale * (re * z.re + im * z.im), scale * (im * z.re - re * z.im)};
    }

    // Hidden friends so a real scalar can appear on the left-hand side.
    /** @brief `value + z`. */
    friend SPECULA_CPU_GPU complex operator+(T value, complex z) { return complex(value) + z; }
    /** @brief `value - z`. */
    friend SPECULA_CPU_GPU complex operator-(T value, complex z) { return complex(value) - z; }
    /** @brief `value * z`. */
    friend SPECULA_CPU_GPU complex operator*(T value, complex z) { return complex(value) * z; }
    /** @brief `value / z`. */
    friend SPECULA_CPU_GPU complex operator/(T value, complex z) { return complex(value) / z; }

    /** @brief Real part. */
    T re;
    /** @brief Imaginary part. */
    T im;
  };

  /** @brief Real part of `z`. @see https://en.cppreference.com/w/cpp/numeric/complex/real2 */
  template <typename T> SPECULA_CPU_GPU T real(const complex<T> &z) { return z.re; }
  /** @brief Imaginary part of `z`. @see https://en.cppreference.com/w/cpp/numeric/complex/imag2 */
  template <typename T> SPECULA_CPU_GPU T imag(const complex<T> &z) { return z.im; }
  /**
   * @brief Squared magnitude @f$ re^2 + im^2 @f$ (the "field norm", not the Euclidean norm).
   * @see https://en.cppreference.com/w/cpp/numeric/complex/norm
   */
  template <typename T> SPECULA_CPU_GPU T norm(const complex<T> &z) {
    return z.re * z.re + z.im * z.im;
  }
  /**
   * @brief Magnitude @f$ |z| = \sqrt{re^2 + im^2} @f$.
   * @note Computed as `sqrt(norm(z))` rather than `hypot`, so it overflows for
   * @f$ |z| \gtrsim \sqrt{T_{max}} @f$.
   * @see https://en.cppreference.com/w/cpp/numeric/complex/abs
   */
  template <typename T> SPECULA_CPU_GPU T abs(const complex<T> &z) {
    return pstd::sqrt(pstd::norm(z));
  }
  /**
   * @brief Principal square root, with the branch cut along the negative real axis.
   *
   * With @f$ t_1 = \sqrt{\tfrac{1}{2}(|z| + |re|)} @f$ and @f$ t_2 = \tfrac{im}{2 t_1} @f$:
   * - @f$ re \ge 0 @f$: @f$ \sqrt{z} = t_1 + i\,t_2 @f$
   * - @f$ re < 0 @f$: @f$ \sqrt{z} = |t_2| + i\,\operatorname{copysign}(t_1, im) @f$
   *
   * so the result always has a non-negative real part and an imaginary part with the sign of
   * `im` (including signed zero), matching `std::sqrt`. Returns `0` for @f$ z = 0 @f$.
   *
   * @note `t2` is evaluated before the zero check, so for @f$ z = 0 @f$ a `0/0` NaN is computed
   * and then discarded (harmless, but may trip FP-exception traps).
   * @see https://en.cppreference.com/w/cpp/numeric/complex/sqrt
   */
  template <typename T> SPECULA_CPU_GPU complex<T> sqrt(const complex<T> &z) {
    T n = pstd::abs(z);
    T t1 = pstd::sqrt(T(0.5) * (n + pstd::abs(z.re)));
    T t2 = T(0.5) * z.im / t1;

    if (n == 0) {
      return 0;
    }
    if (z.re >= 0) {
      return {t1, t2};
    }
    return {pstd::abs(t2), pstd::copysign(t1, z.im)};
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_COMPLEX_HPP
