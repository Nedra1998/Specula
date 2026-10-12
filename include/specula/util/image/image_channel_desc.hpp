#ifndef SPECULA_UTIL_IMAGE_IMAGE_CHANNEL_DESC_HPP
#define SPECULA_UTIL_IMAGE_IMAGE_CHANNEL_DESC_HPP

#include "specula/util/containers.hpp"

namespace specula {
  struct ImageChannelDesc {
    operator bool() const { return size() > 0; }

    [[nodiscard]] size_t size() const { return offset.size(); }
    [[nodiscard]] bool is_identify() const {
      for (size_t i = 0; i < offset.size(); ++i) {
        if (offset[i] != i) {
          return false;
        }
      }
      return true;
    }

    InlinedVector<int, 4> offset;
  };
} // namespace specula

template <> struct fmt::formatter<specula::ImageChannelDesc> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ImageChannelDesc &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ ImageChannelDesc offset={} ]", v.offset);
  }
};

#endif // SPECULA_UTIL_IMAGE_IMAGE_CHANNEL_DESC_HPP
