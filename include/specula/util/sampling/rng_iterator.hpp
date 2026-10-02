#ifndef SPECULA_UTIL_SAMPLING_RNG_ITERATOR_HPP
#define SPECULA_UTIL_SAMPLING_RNG_ITERATOR_HPP

// IWYU pragma: private, include "specula/util/sampling.hpp"

#include <cstdint>

#include "specula/macros.hpp"
#include "specula/util/low_discrepancy.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  namespace detail {
    class Stratified1DIter;
    class Stratified2DIter;
    class Stratified3DIter;
    template <typename Iterator> class RngIterator;

    template <typename Iterator> class IndexingIterator {
    public:
      template <typename Generator>
      SPECULA_CPU_GPU IndexingIterator(int i, int n, const Generator *) : i(i), n(n) {}

      SPECULA_CPU_GPU bool operator==(const Iterator &it) const { return i = it.i; }
      SPECULA_CPU_GPU bool operator!=(const Iterator &it) const { return !(*this == it); }

      SPECULA_CPU_GPU Iterator &operator++() {
        ++i;
        return (Iterator &)*this;
      }
      SPECULA_CPU_GPU Iterator operator++(int) const {
        Iterator it = *this;
        return ++it;
      }

    protected:
      int i, n;
    };

    template <typename Generator, typename Iterator> class IndexingGenerator {
    public:
      SPECULA_CPU_GPU IndexingGenerator(int n) : n(n) {}
      SPECULA_CPU_GPU [[nodiscard]] Iterator begin() const {
        return Iterator(0, n, (const Generator *)this);
      }
      SPECULA_CPU_GPU [[nodiscard]] Iterator end() const {
        return Iterator(n, n, (const Generator *)this);
      }

    protected:
      int n;
    };

    template <typename Generator, typename Iterator>
    class RngGenerator : public IndexingGenerator<Generator, Iterator> {
    public:
      SPECULA_CPU_GPU RngGenerator(int n, uint64_t sequence_index = 0,
                                   uint64_t seed = PCG32_DEFAULT_STATE)
          : IndexingGenerator<Generator, Iterator>(n), sequence_index(sequence_index), seed(seed) {}

    private:
      friend RngIterator<Iterator>;
      uint64_t sequence_index, seed;
    };

    template <typename Iterator> class RngIterator : public IndexingIterator<Iterator> {
    public:
      template <typename Generator>
      SPECULA_CPU_GPU RngIterator(int i, int n, const RngGenerator<Generator, Iterator> *generator)
          : IndexingIterator<Iterator>(i, n, generator), rng(generator->sequence_index) {}

    protected:
      Rng rng;
    };

    class Uniform1DIter : public RngIterator<Uniform1DIter> {
    public:
      using RngIterator<Uniform1DIter>::RngIterator;
      SPECULA_CPU_GPU Float operator*() { return rng.uniform<Float>(); }
    };

    class Uniform2DIter : public RngIterator<Uniform2DIter> {
    public:
      using RngIterator<Uniform2DIter>::RngIterator;
      SPECULA_CPU_GPU Point2f operator*() { return {rng.uniform<Float>(), rng.uniform<Float>()}; }
    };

    class Uniform3DIter : public RngIterator<Uniform3DIter> {
    public:
      using RngIterator<Uniform3DIter>::RngIterator;
      SPECULA_CPU_GPU Point3f operator*() {
        return {rng.uniform<Float>(), rng.uniform<Float>(), rng.uniform<Float>()};
      }
    };

    class Hammersly2DIter : public IndexingIterator<Hammersly2DIter> {
      using IndexingIterator<Hammersly2DIter>::IndexingIterator;
      SPECULA_CPU_GPU Point2f operator*() { return {Float(i) / Float(n), radical_inverse(0, i)}; }
    };

    class Hammersly3DIter : public IndexingIterator<Hammersly3DIter> {
      using IndexingIterator<Hammersly3DIter>::IndexingIterator;
      SPECULA_CPU_GPU Point3f operator*() {
        return {Float(i) / Float(n), radical_inverse(0, i), radical_inverse(1, i)};
      }
    };
  } // namespace detail

  class Uniform1D : public detail::RngGenerator<Uniform1D, detail::Uniform1DIter> {
  public:
    using detail::RngGenerator<Uniform1D, detail::Uniform1DIter>::RngGenerator;
  };

  class Uniform2D : public detail::RngGenerator<Uniform2D, detail::Uniform2DIter> {
  public:
    using detail::RngGenerator<Uniform2D, detail::Uniform2DIter>::RngGenerator;
  };

  class Uniform3D : public detail::RngGenerator<Uniform3D, detail::Uniform3DIter> {
  public:
    using detail::RngGenerator<Uniform3D, detail::Uniform3DIter>::RngGenerator;
  };

  class Hammersley2D : public detail::IndexingGenerator<Hammersley2D, detail::Hammersly2DIter> {
  public:
    using detail::IndexingGenerator<Hammersley2D, detail::Hammersly2DIter>::IndexingGenerator;
  };

  class Hammersley3D : public detail::IndexingGenerator<Hammersley3D, detail::Hammersly3DIter> {
  public:
    using detail::IndexingGenerator<Hammersley3D, detail::Hammersly3DIter>::IndexingGenerator;
  };

  class Stratified1D : public detail::RngGenerator<Stratified1D, detail::Stratified1DIter> {
  public:
    using detail::RngGenerator<Stratified1D, detail::Stratified1DIter>::RngGenerator;
  };

  class Stratified2D : public detail::RngGenerator<Stratified2D, detail::Stratified2DIter> {
  public:
    SPECULA_CPU_GPU Stratified2D(int nx, int ny, uint64_t sequence_index = 0,
                                 uint64_t seed = PCG32_DEFAULT_STATE)
        : detail::RngGenerator<Stratified2D, detail::Stratified2DIter>(nx * ny, sequence_index,
                                                                       seed),
          nx(nx), ny(ny) {}

  private:
    friend detail::Stratified2DIter;
    int nx, ny;
  };

  class Stratified3D : public detail::RngGenerator<Stratified3D, detail::Stratified3DIter> {
  public:
    SPECULA_CPU_GPU Stratified3D(int nx, int ny, int nz, uint64_t sequence_index = 0,
                                 uint64_t seed = PCG32_DEFAULT_STATE)
        : detail::RngGenerator<Stratified3D, detail::Stratified3DIter>(nx * ny * nz, sequence_index,
                                                                       seed),
          nx(nx), ny(ny), nz(nz) {}

  private:
    friend detail::Stratified3DIter;
    int nx, ny, nz;
  };

  namespace detail {
    class Stratified1DIter : public RngIterator<Stratified1DIter> {
      using RngIterator<Stratified1DIter>::RngIterator;
      SPECULA_CPU_GPU Float operator*() { return (i + rng.uniform<Float>()) / n; }
    };

    class Stratified2DIter : public RngIterator<Stratified2DIter> {
      SPECULA_CPU_GPU Stratified2DIter(int i, int n, const Stratified2D *generator)
          : RngIterator<Stratified2DIter>(i, n, generator), nx(generator->nx), ny(generator->ny) {}

      SPECULA_CPU_GPU Point2f operator*() {
        int ix = i % nx, iy = i / nx;
        return {(ix + rng.uniform<Float>()) / nx, (iy + rng.uniform<Float>()) / ny};
      }

    private:
      int nx, ny;
    };

    class Stratified3DIter : public RngIterator<Stratified3DIter> {
      SPECULA_CPU_GPU Stratified3DIter(int i, int n, const Stratified3D *generator)
          : RngIterator<Stratified3DIter>(i, n, generator), nx(generator->nx), ny(generator->ny),
            nz(generator->nz) {}

      SPECULA_CPU_GPU Point3f operator*() {
        int ix = i % nx, iy = (i / nx) % ny, iz = i / (nx * ny);
        return {(ix + rng.uniform<Float>()) / nx, (iy + rng.uniform<Float>()) / ny,
                (iz + rng.uniform<Float>()) / nz};
      }

    private:
      int nx, ny, nz;
    };
  } // namespace detail

} // namespace specula

#endif // SPECULA_UTIL_SAMPLING_RNG_ITERATOR_HPP
