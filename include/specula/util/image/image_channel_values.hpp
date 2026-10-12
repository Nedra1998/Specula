#ifndef SPECULA_UTIL_IMAGE_IMAGE_CHANNEL_VALUES_HPP
#define SPECULA_UTIL_IMAGE_IMAGE_CHANNEL_VALUES_HPP

#include "specula/util/check.hpp"
#include "specula/util/containers.hpp"

namespace specula {
  struct ImageChannelValues : public InlinedVector<Float, 4> {
    explicit ImageChannelValues(size_t sz, Float v = {}) : InlinedVector<Float, 4>(sz, v) {}

    operator Float() const {
      ASSERT_EQ(1, size());
      return (*this)[0];
    }
    operator pstd::array<Float, 3>() const {
      ASSERT_EQ(3, size());
      return {(*this)[0], (*this)[1], (*this)[2]};
    }

    [[nodiscard]] Float max_value() const {
      Float m = (*this)[0];
      for (int i = 1; i < size(); ++i) {
        m = std::max(m, (*this)[i]);
      }
      return m;
    }

    [[nodiscard]] Float average() const {
      Float sum = 0;
      for (int i = 0; i < size(); ++i) {
        sum += (*this)[i];
      }
      return sum / size();
    }
  };
} // namespace specula

template <> struct fmt::formatter<specula::ImageChannelValues> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ImageChannelValues &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ ImageChannelValues {} ]",
                     ((specula::InlinedVector<specula::Float, 4> &)*this));
  }
};

#endif // SPECULA_UTIL_IMAGE_IMAGE_CHANNEL_VALUES_HPP
