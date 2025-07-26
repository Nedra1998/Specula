#ifndef INCLUDE_MATH_SQUARE_MATRIX_HPP_
#define INCLUDE_MATH_SQUARE_MATRIX_HPP_

#include <cstddef>

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/math/functions.hpp"
#include "specula/util/math/interval.hpp"
#include "specula/util/pstd/optional.hpp"
#include "specula/util/pstd/span.hpp"

namespace specula {
  namespace {
    template <std::size_t N>
    SPECULA_CPU_GPU inline void init(Float m[N][N], std::size_t i, std::size_t j) {}

    template <std::size_t N, typename... Args>
    SPECULA_CPU_GPU inline void init(Float m[N][N], std::size_t i, std::size_t j, Float v,
                                     Args... args) {
      m[i][j] = v;
      if (++j == N) {
        ++i;
        j = 0;
      }
      init<N>(m, i, j, args...);
    }

    template <std::size_t N> SPECULA_CPU_GPU inline void init_diag(Float m[N][N], std::size_t i) {}

    template <std::size_t N, typename... Args>
    SPECULA_CPU_GPU inline void init_diag(Float m[N][N], std::size_t i, Float v, Args... args) {
      m[i][i] = v;
      init_diag<N>(m, i + 1, args...);
    }
  } // namespace

  template <std::size_t N> class SquareMatrix {
  public:
    SPECULA_CPU_GPU SquareMatrix() {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          m[i][j] = (i == j) ? 1 : 0;
        }
      }
    }

    SPECULA_CPU_GPU SquareMatrix(const Float mat[N][N]) {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          m[i][j] = mat[i][j];
        }
      }
    }

    template <typename... Args> SPECULA_CPU_GPU SquareMatrix(Float v, Args... args) {
      static_assert(1 + sizeof...(Args) == N * N,
                    "Number of arguments must match N*N for SquareMatrix");
      init<N>(m, 0, 0, v, args...);
    }

    SPECULA_CPU_GPU SquareMatrix(pstd::span<const Float> t) {
      ASSERT_EQ(N * N, t.size());
      for (std::size_t i = 0; i < N * N; ++i) {
        m[i / N][i % N] = t[i];
      }
    }

    SPECULA_CPU_GPU static SquareMatrix zero() {
      SquareMatrix m;
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          m.m[i][j] = 0;
        }
      }
      return m;
    }

    template <typename... Args> SPECULA_CPU_GPU static SquareMatrix diag(Float v, Args... args) {
      static_assert(1 + sizeof...(Args) == N, "Number of arguments must match N for diag");
      SquareMatrix m;
      init_diag<N>(m.m, 0, v, args...);
      return m;
    }

    SPECULA_CPU_GPU SquareMatrix operator+(const SquareMatrix &m) const {
      SquareMatrix r = *this;
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          r.m[i][j] += m.m[i][j];
        }
      }

      return r;
    }

    SPECULA_CPU_GPU SquareMatrix operator*(Float s) const {
      SquareMatrix r = *this;
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          r.m[i][j] *= s;
        }
      }
      return r;
    }

    SPECULA_CPU_GPU SquareMatrix operator/(Float s) const {
      DASSERT_NE(s, 0);
      SquareMatrix r = *this;
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          r.m[i][j] /= s;
        }
      }
      return r;
    }

    SPECULA_CPU_GPU bool operator==(const SquareMatrix<N> &other) const {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          if (m[i][j] != other.m[i][j]) {
            return false;
          }
        }
      }
      return true;
    }

    SPECULA_CPU_GPU bool operator!=(const SquareMatrix<N> &other) const {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          if (m[i][j] != other.m[i][j]) {
            return true;
          }
        }
      }
      return false;
    }

    SPECULA_CPU_GPU bool operator<(const SquareMatrix<N> &other) const {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          if (m[i][j] < other.m[i][j]) {
            return true;
          } else if (m[i][j] > other.m[i][j]) {
            return false;
          }
        }
      }
      return false;
    }

    SPECULA_CPU_GPU inline bool is_identity() const {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
          if (i == j) {
            if (m[i][j] != 1) {
              return false;
            }
          } else if (m[i][j] != 0) {
            return false;
          }
        }
      }
      return true;
    }

    SPECULA_CPU_GPU pstd::span<const Float> operator[](std::size_t i) const { return m[i]; }
    SPECULA_CPU_GPU pstd::span<Float> operator[](std::size_t i) { return pstd::span<Float>(m[i]); }

  private:
    Float m[N][N];

    friend struct fmt::formatter<SquareMatrix<N>>;
  };

  extern template class SquareMatrix<2>;
  extern template class SquareMatrix<3>;
  extern template class SquareMatrix<4>;

  template <std::size_t N>
  SPECULA_CPU_GPU inline SquareMatrix<N> operator*(Float s, const SquareMatrix<N> &m) {
    return m * s;
  }

  template <typename R, std::size_t N, typename T>
  SPECULA_CPU_GPU inline R mul(const SquareMatrix<N> &m, const T &v) {
    R result;
    for (std::size_t i = 0; i < N; ++i) {
      result[i] = 0;
      for (std::size_t j = 0; j < N; ++j) {
        result[i] += m[i][j] * v[j];
      }
    }
    return result;
  }

  template <std::size_t N, typename T>
  SPECULA_CPU_GPU inline T operator*(const SquareMatrix<N> &m, const T &v) {
    return mul<T>(m, v);
  }

  template <std::size_t N>
  SPECULA_CPU_GPU SquareMatrix<N> operator*(const SquareMatrix<N> &m1, const SquareMatrix<N> &m2);

  template <>
  SPECULA_CPU_GPU inline SquareMatrix<4> operator*(const SquareMatrix<4> &m1,
                                                   const SquareMatrix<4> &m2) {
    SquareMatrix<4> r;
    for (std::size_t i = 0; i < 4; ++i) {
      for (std::size_t j = 0; j < 4; ++j) {
        r[i][j] = inner_product(m1[i][0], m2[0][j], m1[i][1], m2[1][j], m1[i][2], m2[2][j],
                                m1[i][3], m2[3][j]);
      }
    }

    return r;
  }

  template <>
  SPECULA_CPU_GPU inline SquareMatrix<3> operator*(const SquareMatrix<3> &m1,
                                                   const SquareMatrix<3> &m2) {
    SquareMatrix<3> r;
    for (std::size_t i = 0; i < 3; ++i) {
      for (std::size_t j = 0; j < 3; ++j) {
        r[i][j] = inner_product(m1[i][0], m2[0][j], m1[i][1], m2[1][j], m1[i][2], m2[2][j]);
      }
    }

    return r;
  }

  template <std::size_t N>
  SPECULA_CPU_GPU inline SquareMatrix<N> operator*(const SquareMatrix<N> &m1,
                                                   const SquareMatrix<N> &m2) {
    SquareMatrix<N> r;
    for (std::size_t i = 0; i < N; ++i) {
      for (std::size_t j = 0; j < N; ++j) {
        r[i][j] = 0;
        for (std::size_t k = 0; k < N; ++k) {
          r[i][j] = fma(m1[i][k], m2[k][j], r[i][j]);
        }
      }
    }

    return r;
  }

  template <std::size_t N> SPECULA_CPU_GPU Float determinant(const SquareMatrix<N> &m);

  template <> SPECULA_CPU_GPU inline Float determinant(const SquareMatrix<1> &m) { return m[0][0]; }
  template <> SPECULA_CPU_GPU inline Float determinant(const SquareMatrix<2> &m) {
    return difference_of_products(m[0][0], m[1][1], m[0][1], m[1][0]);
  }

  template <> SPECULA_CPU_GPU inline Float determinant(const SquareMatrix<3> &m) {
    Float minor12 = difference_of_products(m[1][1], m[2][2], m[1][2], m[2][1]);
    Float minor02 = difference_of_products(m[1][0], m[2][2], m[1][2], m[2][0]);
    Float minor01 = difference_of_products(m[1][0], m[2][1], m[1][1], m[2][0]);
    return fma(m[0][2], minor01, difference_of_products(m[0][0], minor12, m[0][1], minor02));
  }

  template <> SPECULA_CPU_GPU inline Float determinant(const SquareMatrix<4> &m) {
    Float s0 = difference_of_products(m[0][0], m[1][1], m[1][0], m[0][1]);
    Float s1 = difference_of_products(m[0][0], m[1][2], m[1][0], m[0][2]);
    Float s2 = difference_of_products(m[0][0], m[1][3], m[1][0], m[0][3]);

    Float s3 = difference_of_products(m[0][1], m[1][2], m[1][1], m[0][2]);
    Float s4 = difference_of_products(m[0][1], m[1][3], m[1][1], m[0][3]);
    Float s5 = difference_of_products(m[0][2], m[1][3], m[1][2], m[0][3]);

    Float c0 = difference_of_products(m[2][0], m[3][1], m[3][0], m[2][1]);
    Float c1 = difference_of_products(m[2][0], m[3][2], m[3][0], m[2][2]);
    Float c2 = difference_of_products(m[2][0], m[3][3], m[3][0], m[2][3]);

    Float c3 = difference_of_products(m[2][1], m[3][2], m[3][1], m[2][2]);
    Float c4 = difference_of_products(m[2][1], m[3][3], m[3][1], m[2][3]);
    Float c5 = difference_of_products(m[2][2], m[3][3], m[3][2], m[2][3]);

    return (difference_of_products(s0, c5, s1, c4) + difference_of_products(s2, c3, -s3, c2) +
            difference_of_products(s5, c0, s4, c1));
  }

  /**
   * @warn This is inefficent for computing the determinant for larger
   * matrices, however Specula doesn't make use of N>4 matrices at the moment.
   */
  template <std::size_t N> SPECULA_CPU_GPU inline Float determinant(const SquareMatrix<N> &m) {
    SquareMatrix<N - 1> sub;
    Float det = 0;

    for (std::size_t i = 0; i < N; ++i) {
      for (std::size_t j = 0; j < N - 1; ++j) {
        for (std::size_t k = 0; k < N - 1; ++k) {
          sub[j][k] = m[j + 1][k < i ? k : k + 1];
        }
      }
      Float sign = (i & 1) ? -1 : 1;
      det += sign * m[0][i] * determinant(sub);
    }
    return det;
  }

  template <std::size_t N>
  SPECULA_CPU_GPU inline SquareMatrix<N> transpose(const SquareMatrix<N> &m) {
    SquareMatrix<N> r;
    for (std::size_t i = 0; i < N; ++i) {
      for (std::size_t j = 0; j < N; ++j) {
        r[i][j] = m[j][i];
      }
    }
    return r;
  }

  template <std::size_t N>
  SPECULA_CPU_GPU pstd::optional<SquareMatrix<N>> inverse(const SquareMatrix<N> &m);

  template <>
  SPECULA_CPU_GPU inline pstd::optional<SquareMatrix<3>> inverse(const SquareMatrix<3> &m) {
    Float det = determinant(m);
    if (det == 0) {
      return {};
    }

    Float inv_det = 1 / det;
    SquareMatrix<3> r;

    r[0][0] = inv_det * difference_of_products(m[1][1], m[2][2], m[1][2], m[2][1]);
    r[1][0] = inv_det * difference_of_products(m[1][2], m[2][0], m[1][0], m[2][2]);
    r[2][0] = inv_det * difference_of_products(m[1][0], m[2][1], m[1][1], m[2][0]);

    r[0][1] = inv_det * difference_of_products(m[0][2], m[2][1], m[0][1], m[2][2]);
    r[1][1] = inv_det * difference_of_products(m[0][0], m[2][2], m[0][2], m[2][0]);
    r[2][1] = inv_det * difference_of_products(m[0][1], m[2][0], m[0][0], m[2][1]);

    r[0][2] = inv_det * difference_of_products(m[0][1], m[1][2], m[0][2], m[1][1]);
    r[1][2] = inv_det * difference_of_products(m[0][2], m[1][0], m[0][0], m[1][2]);
    r[2][2] = inv_det * difference_of_products(m[0][0], m[1][1], m[0][1], m[1][0]);

    return r;
  }

  template <>
  SPECULA_CPU_GPU inline pstd::optional<SquareMatrix<4>> inverse(const SquareMatrix<4> &m) {
    Float s0 = difference_of_products(m[0][0], m[1][1], m[1][0], m[0][1]);
    Float s1 = difference_of_products(m[0][0], m[1][2], m[1][0], m[0][2]);
    Float s2 = difference_of_products(m[0][0], m[1][3], m[1][0], m[0][3]);

    Float s3 = difference_of_products(m[0][1], m[1][2], m[1][1], m[0][2]);
    Float s4 = difference_of_products(m[0][1], m[1][3], m[1][1], m[0][3]);
    Float s5 = difference_of_products(m[0][2], m[1][3], m[1][2], m[0][3]);

    Float c0 = difference_of_products(m[2][0], m[3][1], m[3][0], m[2][1]);
    Float c1 = difference_of_products(m[2][0], m[3][2], m[3][0], m[2][2]);
    Float c2 = difference_of_products(m[2][0], m[3][3], m[3][0], m[2][3]);

    Float c3 = difference_of_products(m[2][1], m[3][2], m[3][1], m[2][2]);
    Float c4 = difference_of_products(m[2][1], m[3][3], m[3][1], m[2][3]);
    Float c5 = difference_of_products(m[2][2], m[3][3], m[3][2], m[2][3]);

    Float determinant = inner_product(s0, c5, -s1, c4, s2, c3, s3, c2, s5, c0, -s4, c1);
    if (determinant == 0) {
      return {};
    }

    Float s = 1 / determinant;

    Float inv[4][4] = {{s * inner_product(m[1][1], c5, m[1][3], c3, -m[1][2], c4),
                        s * inner_product(-m[0][1], c5, m[0][2], c4, -m[0][3], c3),
                        s * inner_product(m[3][1], s5, m[3][3], s3, -m[3][2], s4),
                        s * inner_product(-m[2][1], s5, m[2][2], s4, -m[2][3], s3)},

                       {s * inner_product(-m[1][0], c5, m[1][2], c2, -m[1][3], c1),
                        s * inner_product(m[0][0], c5, m[0][3], c1, -m[0][2], c2),
                        s * inner_product(-m[3][0], s5, m[3][2], s2, -m[3][3], s1),
                        s * inner_product(m[2][0], s5, m[2][3], s1, -m[2][2], s2)},

                       {s * inner_product(m[1][0], c4, m[1][3], c0, -m[1][1], c2),
                        s * inner_product(-m[0][0], c4, m[0][1], c2, -m[0][3], c0),
                        s * inner_product(m[3][0], s4, m[3][3], s0, -m[3][1], s2),
                        s * inner_product(-m[2][0], s4, m[2][1], s2, -m[2][3], s0)},

                       {s * inner_product(-m[1][0], c3, m[1][1], c1, -m[1][2], c0),
                        s * inner_product(m[0][0], c3, m[0][2], c0, -m[0][1], c1),
                        s * inner_product(-m[3][0], s3, m[3][1], s1, -m[3][2], s0),
                        s * inner_product(m[2][0], s3, m[2][2], s0, -m[2][1], s1)}};

    return SquareMatrix<4>(inv);
  }

  template <std::size_t N>
  SPECULA_CPU_GPU inline SquareMatrix<N> inverse_or_exit(const SquareMatrix<N> &m) {
    pstd::optional<SquareMatrix<N>> inv = inverse(m);
    ASSERT(inv.has_value());
    return *inv;
  }

} // namespace specula

template <std::size_t N> struct fmt::formatter<specula::SquareMatrix<N>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::SquareMatrix<N> &v, FormatContext &ctx) const {
    format_to(ctx.out(), "[ ");
    for (std::size_t i = 0; i < N; ++i) {
      format_to(ctx.out(), "[ ");
      for (std::size_t j = 0; j < N; ++j) {
        format_to(ctx.out(), "{}{}", v.m[i][j], j < N - 1 ? ", " : " ");
      }
      format_to(ctx.out(), "{}", i < N - 1 ? "], " : "] ");
    }
    return format_to(ctx.out(), "]");
  }
};

#endif // INCLUDE_MATH_SQUARE_MATRIX_HPP_
