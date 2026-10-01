#include "specula/util/sampling/alias_table.hpp"

#include <numeric>

#include "specula/util/float.hpp"

specula::AliasTable::AliasTable(pstd::span<const Float> weights, Allocator alloc)
    : bins(weights.size(), alloc) {
  Float sum = std::accumulate(weights.begin(), weights.end(), 0.0);
  ASSERT_GT(sum, 0);
  for (size_t i = 0; i < weights.size(); ++i) {
    bins[i].p = weights[i] / sum;
  }

  struct Outcome {
    Float phat;
    size_t index;
  };
  std::vector<Outcome> under, over;

  for (size_t i = 0; i < bins.size(); ++i) {
    Float phat = bins[i].p * bins.size();
    if (phat < 1) {
      under.push_back(Outcome{.phat = phat, .index = i});
    } else {
      over.push_back(Outcome{.phat = phat, .index = i});
    }
  }

  while (!under.empty() && !over.empty()) {
    Outcome un = under.back(), ov = over.back();
    under.pop_back();
    over.pop_back();

    bins[un.index].q = un.phat;
    bins[un.index].alias = ov.index;

    Float pexcess = un.phat + ov.phat - 1;
    if (pexcess < 1) {
      under.push_back(Outcome{.phat = pexcess, .index = ov.index});
    } else {
      over.push_back(Outcome{.phat = pexcess, .index = ov.index});
    }
  }

  while (!over.empty()) {
    Outcome ov = over.back();
    over.pop_back();
    bins[ov.index].q = 1;
    bins[ov.index].alias = -1;
  }

  while (!under.empty()) {
    Outcome un = under.back();
    under.pop_back();
    bins[un.index].q = 1;
    bins[un.index].alias = -1;
  }
}

SPECULA_CPU_GPU int specula::AliasTable::sample(Float u, Float *pmf, Float *uremapped) const {
  int offset = std::min<int>(u * bins.size(), bins.size() - 1);
  Float up = std::min<Float>(u * bins.size() - offset, ONE_MINUS_EPSILON);

  if (up < bins[offset].q) {
    DASSERT_GT(bins[offset].p, 0);
    if (pmf != nullptr) {
      *pmf = bins[offset].p;
    }
    if (uremapped != nullptr) {
      *uremapped = std::min<Float>(up / bins[offset].q, ONE_MINUS_EPSILON);
    }
    return offset;
  } else {
    int alias = bins[offset].alias;
    DASSERT_GE(alias, 0);
    DASSERT_GT(bins[alias].p, 0);
    if (pmf != nullptr) {
      *pmf = bins[alias].p;
    }
    if (uremapped != nullptr) {
      *uremapped = std::min<Float>((up - bins[offset].p) / (1 - bins[offset].q), ONE_MINUS_EPSILON);
    }
    return alias;
  }
}
