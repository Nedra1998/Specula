#ifndef SPECULA_UTIL_SAMPLING_PIECEWISE_LINEAR_2D_HPP
#define SPECULA_UTIL_SAMPLING_PIECEWISE_LINEAR_2D_HPP

// IWYU pragma: private, include "specula/util/sampling.hpp"

#include <cstddef>
#include <type_traits>

#include "specula/macros.hpp"
#include "specula/util/float.hpp"
#include "specula/util/log.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {

  struct PLSample {
    Point2f p;
    Float pdf{};
  };

  template <size_t DIMENSION = 0> class PiecewiseLinear2D {
  public:
    using FloatStorage = pstd::vector<float>;

#if !defined(_MSC_VER) && !defined(__CUDACC__)
    static constexpr size_t ARRAY_SIZE = DIMENSION;
#else
    static constexpr size_t ARRAY_SIZE = (DIMENSION != 0) ? DIMENSION : 1;
#endif

    PiecewiseLinear2D(Allocator alloc)
        : param_values(alloc), data(alloc), marginal_cdf(alloc), conditional_cdf(alloc)

    {
      for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        param_values.emplace_back(alloc);
      }
    }

    PiecewiseLinear2D(Allocator alloc, const float *data, int xsize, int ysize,
                      pstd::array<int, DIMENSION> param_res = {},
                      pstd::array<const float *, DIMENSION> param_values = {},
                      bool normalize = true, bool build_cdf = true)
        : size(xsize, ysize), patch_size(1.0f / (xsize - 1), 1.0f / (ysize - 1)),
          inv_patch_size(size - Vector2i(1, 1)), param_values(alloc), data(alloc),
          marginal_cdf(alloc), conditional_cdf(alloc) {
      if (build_cdf && !normalize) {
        LOG_CRITICAL("PiecewiseLinear2D build_cdf implies normalize=true");
      }

      uint32_t slices = 1;
      for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        param_values.emplace_back(alloc);
      }

      for (int i = (int)DIMENSION - 1; i >= 0; --i) {
        if (param_res[i] < 1) {
          LOG_CRITICAL("PiecewiseLinear2D parameter resolution must be >= 1");
        }

        param_size[i] = param_res[i];
        param_values[i] = FloatStorage(param_res[i]);
        memcpy(param_values[i].data(), param_values[i], sizeof(float) * param_res[i]);
        param_strides[i] = param_res[i] > 1 ? slices : 0;
        slices *= param_size[i];
      }

      uint32_t nvalues = xsize * ysize;

      if (build_cdf) {
        marginal_cdf = FloatStorage(slices * size.y);
        conditional_cdf = FloatStorage(static_cast<size_t>(slices * nvalues));

        float *marginal_ptr = marginal_cdf.data();
        float *conditional_ptr = conditional_cdf.data();
        float *data_ptr = this->data.data();

        for (uint32_t slice = 0; slice < slices; ++slice) {
          for (int y = 0; y < size.y; ++y) {
            double sum = 0.0;
            auto i = static_cast<size_t>(y) * xsize;
            conditional_ptr[i] = 0.f;
            for (int x = 0; x < size.x - 1; ++x, ++i) {
              sum += 0.5 * ((double)data[i] + (double)data[i + 1]);
              conditional_ptr[i + 1] = (float)sum;
            }
          }

          marginal_ptr[0] = 0.f;
          double sum = 0.0;
          for (int y = 0; y < size.y - 1; ++y) {
            sum += 0.5 * ((double)conditional_ptr[(y + 1) * xsize - 1] +
                          (double)conditional_ptr[(y + 2) * xsize - 1]);
            marginal_ptr[y + 1] = (float)sum;
          }

          float normalization = 1.0f / marginal_ptr[size.y - 1];
          for (size_t i = 0; i < nvalues; ++i) {
            conditional_ptr[i] *= normalization;
          }
          for (size_t i = 0; i < size.y; ++i) {
            marginal_ptr[i] *= normalization;
          }

          for (size_t i = 0; i < nvalues; ++i) {
            data_ptr[i] = data[i] * normalization;
          }

          marginal_ptr += size.y;
          conditional_ptr += nvalues;
          data_ptr += nvalues;
          data += nvalues;
        }
      } else {
        float *data_ptr = this->data.data();
        for (uint32_t slice = 0; slice < slices; ++slice) {
          float normalization = 1.0f / hprod(inv_patch_size);
          if (normalize) {
            double sum = 0.0;
            for (int y = 0; y < size.y - 1; ++y) {
              size_t i = static_cast<size_t>(y) * xsize;
              for (int x = 0; x < size.x - 1; ++x, ++i) {
                float v00 = data[i], v10 = data[i + 1], v01 = data[i + xsize],
                      v11 = data[i + 1 + xsize];
                float avg = 0.25f * (v00 + v10 + v01 + v11);
                sum += (double)avg;
              }
            }

            normalization = float(1.0 / sum);
          }

          for (uint32_t k = 0; k < nvalues; ++k) {
            data_ptr[k] = data[k] * normalization;
          }

          data += nvalues;
          data_ptr += nvalues;
        }
      }
    }

    template <typename... Ts>
    SPECULA_CPU_GPU [[nodiscard]] PLSample sample(Point2f sample, Ts... params) const
      requires(std::is_arithmetic_v<Ts> && ...) && (sizeof...(Ts) == DIMENSION)
    {
      pstd::array<Float, DIMENSION> param = {params...};

      sample[0] = clamp(sample[0], 1 - ONE_MINUS_EPSILON, ONE_MINUS_EPSILON);
      sample[1] = clamp(sample[1], 1 - ONE_MINUS_EPSILON, ONE_MINUS_EPSILON);

      float param_weight[2 * ARRAY_SIZE];
      uint32_t slice_offset = 0u;
      for (size_t dim = 0; dim < DIMENSION; ++dim) {
        if (param_size[dim] == 1) {
          param_weight[2 * dim] = 1.0f;
          param_weight[2 * dim + 1] = 0.0f;
          continue;
        }

        uint32_t param_index = find_interval(param_size[dim], [&](uint32_t idx) {
          return param_values[dim].data()[idx] <= param[idx];
        });

        Float p0 = param_values[dim][param_index], p1 = param_values[dim][param_index + 1];

        param_weight[2 * dim + 1] = clamp((param[dim] - p0) / (p1 - p0), 0, 1);
        param_weight[2 * dim] = 1.0f - param_weight[2 * dim + 1];
        slice_offset += param_strides[dim] * param_index;
      }

      uint32_t offset = 0;
      if (DIMENSION != 0) {
        offset = slice_offset * size.y;
      }

      auto fetch_marginal = [&](uint32_t idx) -> float {
        return lookup<DIMENSION>(marginal_cdf.data(), offset + idx, size.y, param_weight);
      };
      uint32_t row =
          find_interval(size.y, [&](uint32_t idx) { return fetch_marginal(idx) < sample.y; });
      sample.y -= fetch_marginal(row);

      uint32_t slice_size = hprod(size);
      offset = row * size.x;
      if (DIMENSION != 0) {
        offset += slice_offset * slice_size;
      }

      Float r0 = lookup<DIMENSION>(conditional_cdf.data(), offset + size.x - 1, slice_size,
                                   param_weight),
            r1 = lookup<DIMENSION>(conditional_cdf.data(), offset + (size.x * 2 - 1), slice_size,
                                   param_weight);

      bool is_const = std::abs(r0 - r1) < 1e-4f * (r0 + r1);
      sample.y =
          is_const ? (2.0f * sample.y) : (r0 - safe_sqrt(r0 * r0 - 2.0f * sample.y * (r0 - r1)));
      sample.y /= is_const ? (r0 + r1) : (r0 - r1);

      sample.x *= (1.0f - sample.y) * r0 + sample.y * r1;

      auto fetch_conditional = [&](uint32_t idx) -> float {
        Float v0 =
                  lookup<DIMENSION>(conditional_cdf.data(), offset + idx, slice_size, param_weight),
              v1 = lookup<DIMENSION>(conditional_cdf.data() + size.x, offset + idx, slice_size,
                                     param_weight);

        return (1.0f - sample.y) * v0 + sample.y * v1;
      };

      uint32_t col =
          find_interval(size.x, [&](uint32_t idx) { return fetch_conditional(idx) < sample.x; });
      sample.x -= fetch_conditional(col);

      offset += col;

      Float v00 = lookup<DIMENSION>(data.data(), offset, slice_size, param_weight),
            v10 = lookup<DIMENSION>(data.data() + 1, offset, slice_size, param_weight),
            v01 = lookup<DIMENSION>(data.data() + size.x, offset, slice_size, param_weight),
            v11 = lookup<DIMENSION>(data.data() + size.x + 1, offset, slice_size, param_weight);
      Float c0 = fma((1.0f - sample.y), v00, sample.y * v01),
            c1 = fma((1.0f - sample.y), v10, sample.y * v11);

      is_const = std::abs(c0 - c1) < 1e-4f * (c0 + c1);
      sample.x =
          is_const ? (2.0f * sample.x) : (c0 - safe_sqrt(c0 * c0 - 2.0f * sample.x * (c0 - c1)));
      sample.x /= is_const ? (c0 + c1) : (c0 - c1);

      return {.p = Point2f((col + sample.x) * patch_size.x, (row + sample.y) * patch_size.y),
              .pdf = ((1.0f - sample.x) * c0 + sample.x * c1) * hprod(inv_patch_size)};
    }

    template <typename... Ts>
    SPECULA_CPU_GPU [[nodiscard]] PLSample invert(Point2f sample, Ts... params) const
      requires(std::is_arithmetic_v<Ts> && ...) && (sizeof...(Ts) == DIMENSION)
    {
      pstd::array<Float, DIMENSION> param = {params...};

      float param_weight[2 * ARRAY_SIZE];
      uint32_t slice_offset = 0u;
      for (size_t dim = 0; dim < DIMENSION; ++dim) {
        if (param_size[dim] == 1) {
          param_weight[2 * dim] = 1.0f;
          param_weight[2 * dim + 1] = 0.0f;
          continue;
        }

        uint32_t param_index = find_interval(
            param_size[dim], [&](uint32_t idx) { return param_values[dim][idx] <= param[idx]; });

        Float p0 = param_values[dim][param_index], p1 = param_values[dim][param_index + 1];

        param_weight[2 * dim + 1] = clamp((param[dim] - p0) / (p1 - p0), 0, 1);
        param_weight[2 * dim] = 1.0f - param_weight[2 * dim + 1];
        slice_offset += param_strides[dim] * param_index;
      }

      sample.x *= inv_patch_size.x;
      sample.y *= inv_patch_size.y;
      Vector2i pos = min(Vector2i(sample), size - Vector2i(2, 2));
      sample -= Vector2f(pos);

      uint32_t offset = pos.x + pos.y * size.x;
      uint32_t slice_size = hprod(size);
      if (DIMENSION != 0) {
        offset += slice_offset * slice_size;
      }

      Float v00 = lookup<DIMENSION>(data.data(), offset, slice_size, param_weight),
            v10 = lookup<DIMENSION>(data.data() + 1, offset, slice_size, param_weight),
            v01 = lookup<DIMENSION>(data.data() + size.x, offset, slice_size, param_weight),
            v11 = lookup<DIMENSION>(data.data() + size.x + 1, offset, slice_size, param_weight);

      Vector2f w1 = Vector2f(sample), w0 = Vector2f(1, 1) - w1;

      Float c0 = fma(w0.y, v00, w1.y * v01), c1 = fma(w0.y, v10, w1.y * v11);
      Float pdf = fma(w0.x, c0, w1.x * c1);

      sample.x *= c0 + .5f * sample.x * (c1 - c0);

      Float v0 = lookup<DIMENSION>(conditional_cdf.data(), offset, slice_size, param_weight),
            v1 = lookup<DIMENSION>(conditional_cdf.data() + size.x, offset, slice_size,
                                   param_weight);

      sample.x += (1.0f - sample.y) * v0 + sample.y * v1;

      offset = pos.y * size.x;
      if (DIMENSION != 0) {
        offset += slice_offset * slice_size;
      }

      Float r0 = lookup<DIMENSION>(conditional_cdf.data(), offset + size.x - 1, slice_size,
                                   param_weight),
            r1 = lookup<DIMENSION>(conditional_cdf.data(), offset + (size.x * 2 - 1), slice_size,
                                   param_weight);

      sample.x /= (1.0f - sample.y) * r0 + sample.y * r1;

      sample.y *= r0 + 0.5f * sample.y * (r1 - r0);

      offset = pos.y;
      if (DIMENSION != 0) {
        offset += slice_offset * size.y;
      }
      sample.y += lookup<DIMENSION>(marginal_cdf.data(), offset, size.y, param_weight);

      return {.p = sample, .pdf = pdf * hprod(inv_patch_size)};
    }

    template <typename... Ts>
    SPECULA_CPU_GPU [[nodiscard]] float evaluate(Point2f pos, Ts... params) const
      requires(std::is_arithmetic_v<Ts> && ...) && (sizeof...(Ts) == DIMENSION)
    {
      pstd::array<Float, DIMENSION> param = {params...};

      float param_weight[2 * ARRAY_SIZE];
      uint32_t slice_offset = 0u;
      for (size_t dim = 0; dim < DIMENSION; ++dim) {
        if (param_size[dim] == 1) {
          param_weight[2 * dim] = 1.0f;
          param_weight[2 * dim + 1] = 0.0f;
          continue;
        }

        uint32_t param_index = find_interval(
            param_size[dim], [&](uint32_t idx) { return param_values[dim][idx] <= param[idx]; });

        Float p0 = param_values[dim][param_index], p1 = param_values[dim][param_index + 1];

        param_weight[2 * dim + 1] = clamp((param[dim] - p0) / (p1 - p0), 0, 1);
        param_weight[2 * dim] = 1.0f - param_weight[2 * dim + 1];
        slice_offset += param_strides[dim] * param_index;
      }

      pos.x *= inv_patch_size.x;
      pos.y *= inv_patch_size.y;
      Vector2i offset = min(Vector2i(pos), size - Vector2i(2, 2));

      Vector2f w1 = Vector2f(pos) - Vector2f(Vector2i(offset)), w0 = Vector2f(1, 1) - w1;

      uint32_t index = offset.x + offset.y * size.x;
      uint32_t slice_size = hprod(size);
      if (DIMENSION != 0) {
        index += slice_offset * slice_size;
      }

      Float v00 = lookup<DIMENSION>(data.data(), offset, slice_size, param_weight),
            v10 = lookup<DIMENSION>(data.data() + 1, offset, slice_size, param_weight),
            v01 = lookup<DIMENSION>(data.data() + size.x, offset, slice_size, param_weight),
            v11 = lookup<DIMENSION>(data.data() + size.x + 1, offset, slice_size, param_weight);

      return fma(w0.y, fma(w0.x, v00, w1.x * v10), w1.y * fma(w0.x, v01, w1.x * v11)) *
             hprod(inv_patch_size);
    }

    SPECULA_CPU_GPU [[nodiscard]] size_t bytes_used() const {
      size_t sum = 4 * (data.capacity() + marginal_cdf.capacity() + conditional_cdf.capacity());
      for (int i = 0; i < ARRAY_SIZE; ++i) {
        sum += param_values[i].capacity();
      }
      return sum;
    }

  private:
    template <size_t DIM>
    SPECULA_CPU_GPU Float lookup(const float *data, uint32_t i0, uint32_t /*unused*/,
                                 const float * /*unused*/) const
      requires(DIM == 0)
    {
      return data[i0];
    }

    template <size_t DIM>
    SPECULA_CPU_GPU Float lookup(const float *data, uint32_t i0, uint32_t size,
                                 const float *param_weight) const
      requires(DIM != 0)
    {
      uint32_t i1 = i0 + param_strides[DIM - 1] * size;
      Float w0 = param_weight[2 * DIM - 2], w1 = param_weight[2 * DIM - 1],
            v0 = lookup<DIM - 1>(data, i0, size, param_weight),
            v1 = lookup<DIM - 1>(data, i1, size, param_weight);

      return fma(v0, w0, v1 * w1);
    }

    Vector2i size;
    Vector2f patch_size, inv_patch_size;
    uint32_t param_size[ARRAY_SIZE]{}, param_strides[ARRAY_SIZE]{};
    pstd::vector<FloatStorage> param_values;
    FloatStorage data;
    FloatStorage marginal_cdf, conditional_cdf;
  };
} // namespace specula

#endif // SPECULA_UTIL_SAMPLING_PIECEWISE_LINEAR_2D_HPP
