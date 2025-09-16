#ifndef INCLUDE_IMAGE_IMAGE_CHANNEL_HPP_
#define INCLUDE_IMAGE_IMAGE_CHANNEL_HPP_

#include "specula.hpp"
#include "specula/util/containers/inlined_vector.hpp"
#include "util/check.hpp"
#include "util/pstd/array.hpp"

namespace specula {
  struct ImageChannelDesc {
    bool is_identity() const {
      for (size_t i = 0; i < offset.size(); ++i) {
        if (offset[i] != i) {
          return false;
        }
      }
      return true;
    }

    size_t size() const { return offset.size(); }

    operator bool() const { return size() > 0; }

    InlinedVector<int, 4> offset;
  };

  struct ImageChannelValues : public InlinedVector<Float, 4> {
    explicit ImageChannelValues(size_t sz, Float v = {}) : InlinedVector<Float, 4>(sz, v) {}

    Float max_value() const {
      Float m = (*this)[0];
      for (int i = 1; i < size(); ++i) {
        m = std::max(m, (*this)[i]);
      }
      return m;
    }

    Float average() const {
      Float sum = 0;
      for (int i = 0; i < size(); ++i) {
        sum += (*this)[i];
      }
      return sum / size();
    }

    operator Float() const {
      ASSERT_EQ(1, size());
      return (*this)[0];
    }

    operator pstd::array<Float, 3>() const {
      ASSERT_EQ(3, size());
      return {(*this)[0], (*this)[1], (*this)[2]};
    }
  };

} // namespace specula

#endif // INCLUDE_IMAGE_IMAGE_CHANNEL_HPP_
