#ifndef SPECULA_UTIL_CONTAINERS_SAMPLED_GRID_HPP
#define SPECULA_UTIL_CONTAINERS_SAMPLED_GRID_HPP

#include <cstddef>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  template <typename T> class SampledGrid {
  public:
    using const_iterator = typename pstd::vector<T>::const_iterator;

    SampledGrid() = default;
    SampledGrid(Allocator alloc) : values(alloc) {}
    SampledGrid(pstd::span<const T> v, int nx, int ny, int nz, Allocator alloc)
        : values(v.begin(), v.end(), alloc), nx(nx), ny(ny), nz(nz) {
      ASSERT_EQ(nx * ny * nz, values.size());
    }

    SPECULA_CPU_GPU [[nodiscard]] size_t bytes_allocated() const {
      return values.size() * sizeof(T);
    }
    SPECULA_CPU_GPU [[nodiscard]] int xsize() const { return nx; }
    SPECULA_CPU_GPU [[nodiscard]] int ysize() const { return ny; }
    SPECULA_CPU_GPU [[nodiscard]] int zsize() const { return nz; }

    const_iterator begin() const { return values.begin(); }
    const_iterator end() const { return values.end(); }

    template <typename F> SPECULA_CPU_GPU auto lookup(Point3f p, F convert) const {
      Point3f p_samples(p.x * nx - 0.5f, p.y * ny - 0.5f, p.z * nz - 0.5f);
      auto pi = (Point3i)floor(p_samples);
      Vector3f d = p_samples - (Point3f)pi;

      auto d00 = lerp(d.x, lookup(pi, convert), lookup(pi + Vector3i(1, 0, 0), convert));
      auto d10 = lerp(d.x, lookup(pi + Vector3i(0, 1, 0), convert),
                      lookup(pi + Vector3i(1, 1, 0), convert));
      auto d01 = lerp(d.x, lookup(pi + Vector3i(0, 0, 1), convert),
                      lookup(pi + Vector3i(1, 0, 1), convert));
      auto d11 = lerp(d.x, lookup(pi + Vector3i(0, 1, 1), convert),
                      lookup(pi + Vector3i(1, 1, 1), convert));

      return lerp(d.z, lerp(d.y, d00, d10), lerp(d.y, d01, d11));
    }

    SPECULA_CPU_GPU T lookup(Point3f p) const {
      Point3f p_samples(p.x * nx - 0.5f, p.y * ny - 0.5f, p.z * nz - 0.5f);
      auto pi = (Point3i)floor(p_samples);
      Vector3f d = p_samples - (Point3f)pi;

      auto d00 = lerp(d.x, lookup(pi), lookup(pi + Vector3i(1, 0, 0)));
      auto d10 = lerp(d.x, lookup(pi + Vector3i(0, 1, 0)), lookup(pi + Vector3i(1, 1, 0)));
      auto d01 = lerp(d.x, lookup(pi + Vector3i(0, 0, 1)), lookup(pi + Vector3i(1, 0, 1)));
      auto d11 = lerp(d.x, lookup(pi + Vector3i(0, 1, 1)), lookup(pi + Vector3i(1, 1, 1)));

      return lerp(d.z, lerp(d.y, d00, d10), lerp(d.y, d01, d11));
    }

    template <typename F> SPECULA_CPU_GPU auto lookup(const Point3i &p, F convert) const {
      Bounds3i sample_bounds(Point3i(0, 0, 0), Point3i(nx, ny, nz));
      if (!inside_exclusive(p, sample_bounds)) {
        return convert(T{});
      }
      return convert(values[(p.z * ny + p.y) * nx + p.x]);
    }

    SPECULA_CPU_GPU T lookup(const Point3i &p) const {
      Bounds3i sample_bounds(Point3i(0, 0, 0), Point3i(nx, ny, nz));
      if (!inside_exclusive(p, sample_bounds)) {
        return T{};
      }
      return values[(p.z * ny + p.y) * nx + p.x];
    }

    template <typename F> auto max_value(const Bounds3f &bounds, F convert) const {
      Point3f ps[2] = {
          Point3f(bounds.p_min.x * nx - 0.5f, bounds.p_min.y * ny - 0.5f,
                  bounds.p_min.z * nz - 0.5f),
          Point3f(bounds.p_max.x * nx - 0.5f, bounds.p_max.y * ny - 0.5f,
                  bounds.p_max.z * nz - 0.5f),
      };

      Point3i pi[2] = {
          max(Point3i(floor(ps[0])), Point3i(0, 0, 0)),
          min(Point3i(floor(ps[1])) + Vector3i(1, 1, 1), Point3i(nx - 1, ny - 1, nz - 1)),
      };

      auto max_value = lookup(Point3i(pi[0]), convert);
      for (int z = pi[0].z; z <= pi[1].z; ++z) {
        for (int y = pi[0].y; y <= pi[1].y; ++y) {
          for (int x = pi[0].x; x <= pi[1].x; ++x) {
            max_value = std::max(max_value, lookup(Point3i(x, y, z), convert));
          }
        }
      }

      return max_value;
    }

    T max_value(const Bounds3f &bounds) const {
      return max_value(bounds, [](T value) { return value; });
    }

  private:
    pstd::vector<T> values;
    int nx{}, ny{}, nz{};

    friend struct fmt::formatter<SampledGrid<T>>;
  };
} // namespace specula

template <typename T> struct fmt::formatter<specula::SampledGrid<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::SampledGrid<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ SampledGrid nx={} ny={} nz={} values={}]", v.nx, v.ny, v.nz,
                     v.values);
  }
};

#endif // SPECULA_UTIL_CONTAINERS_SAMPLED_GRID_HPP
