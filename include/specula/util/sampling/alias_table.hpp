#ifndef SPECULA_UTIL_SAMPLING_ALIAS_TABLE_HPP
#define SPECULA_UTIL_SAMPLING_ALIAS_TABLE_HPP

// IWYU pragma: private, include "specula/util/sampling.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/pstd.hpp"

namespace specula {
  class AliasTable {
  public:
    AliasTable() = default;
    AliasTable(Allocator alloc = {}) : bins(alloc) {}
    AliasTable(pstd::span<const Float> weights, Allocator alloc = {});

    SPECULA_CPU_GPU int sample(Float u, Float *pmf = nullptr, Float *uremapped = nullptr) const;
    SPECULA_CPU_GPU [[nodiscard]] size_t size() const { return bins.size(); }
    SPECULA_CPU_GPU [[nodiscard]] Float pmf(int index) const { return bins[index].p; }

  private:
    struct Bin {
      Float q, p;
      int alias;
    };

    pstd::vector<Bin> bins;

    friend struct fmt::formatter<AliasTable>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::AliasTable::Bin> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::AliasTable::Bin &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ Bin q={} p={} alias={} ]", v.q, v.p, v.alias);
  }
};

template <> struct fmt::formatter<specula::AliasTable> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::AliasTable &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ AliasTable bins={} ]", v.bins);
  }
};

#endif // SPECULA_UTIL_SAMPLING_ALIAS_TABLE_HPP
