#include "util/math/square_matrix.hpp"

namespace specula {
  template class SquareMatrix<2>;
  template class SquareMatrix<3>;
  template class SquareMatrix<4>;

  template SPECULA_CPU_GPU pstd::optional<SquareMatrix<2>> inverse(const SquareMatrix<2> &);
  template SPECULA_CPU_GPU SquareMatrix<2> operator*(const SquareMatrix<2> &m1,
                                                     const SquareMatrix<2> &m2);
} // namespace specula

template <std::size_t N>
SPECULA_CPU_GPU specula::pstd::optional<specula::SquareMatrix<N>>
specula::inverse(const SquareMatrix<N> &m) {
  std::size_t indxc[N], indxr[N];
  std::size_t ipiv[N] = {0};

  Float minv[N][N];

  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = 0; j < N; ++j) {
      minv[i][j] = m[i][j];
    }
  }

  for (std::size_t i = 0; i < N; ++i) {
    std::size_t irow = 0, icol = 0;
    Float big = 0.0f;

    for (std::size_t j = 0; j < N; ++j) {
      if (ipiv[j] != 1) {
        for (std::size_t k = 0; k < N; ++k) {
          if (ipiv[k] == 0) {
            if (std::abs(minv[j][k]) >= big) {
              big = std::abs(minv[j][k]);
              irow = j;
              icol = k;
            }
          } else if (ipiv[k] > 1) {
            return {}; // Matrix is signular
          }
        }
      }
    }

    ++ipiv[icol];
    if (irow != icol) {
      for (std::size_t k = 0; k < N; ++k) {
        pstd::swap(minv[irow][k], minv[icol][k]);
      }
    }

    indxr[i] = irow;
    indxc[i] = icol;
    if (minv[icol][icol] == 0.0f) {
      return {}; // Matrix is signular
    }

    Float pivinv = 1.0 / minv[icol][icol];
    minv[icol][icol] = 1.0;
    for (std::size_t j = 0; j < N; ++j) {
      minv[icol][j] *= pivinv;
    }

    for (std::size_t j = 0; j < N; ++j) {
      if (j != icol) {
        Float save = minv[j][icol];
        minv[j][icol] = 0;
        for (std::size_t k = 0; k < N; ++k) {
          minv[j][k] = fma(-minv[icol][k], save, minv[j][k]);
        }
      }
    }
  }

  for (int j = N - 1; j >= 0; --j) {
    if (indxr[j] != indxc[j]) {
      for (std::size_t k = 0; k < N; ++k) {
        pstd::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
      }
    }
  }
  return SquareMatrix<N>(minv);
}
