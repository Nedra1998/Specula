#ifndef INCLUDE_UTIL_TAGGED_POINTER_HPP_
#define INCLUDE_UTIL_TAGGED_POINTER_HPP_

#include <type_traits>

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/containers/type_pack.hpp"

namespace specula {

  namespace detail {
    template <typename F, typename R, typename T>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_EQ(0, index);
      return func((const T *)ptr);
    }

    template <typename F, typename R, typename T>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_EQ(0, index);
      return func((T *)ptr);
    }

    template <typename F, typename R, typename T0, typename T1>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 2);

      if (index == 0) {
        return func((const T0 *)ptr);
      } else {
        return func((const T1 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 2);

      if (index == 0) {
        return func((T0 *)ptr);
      } else {
        return func((T1 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 3);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      default:
        return func((const T2 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 3);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      default:
        return func((T2 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 4);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      default:
        return func((const T3 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 4);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      default:
        return func((T3 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 5);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      default:
        return func((const T4 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 5);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      default:
        return func((T4 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 6);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      default:
        return func((const T5 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 6);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      default:
        return func((T5 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 7);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      case 5:
        return func((const T5 *)ptr);
      default:
        return func((const T6 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 7);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      case 5:
        return func((T5 *)ptr);
      default:
        return func((T6 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 8);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      case 5:
        return func((const T5 *)ptr);
      case 6:
        return func((const T6 *)ptr);
      default:
        return func((const T7 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 8);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      case 5:
        return func((T5 *)ptr);
      case 6:
        return func((T6 *)ptr);
      default:
        return func((T7 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7, typename... Ts,
              typename = typename std::enable_if_t<(sizeof...(Ts) > 0)>>
    SPECULA_CPU_GPU R dispatch(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      case 5:
        return func((const T5 *)ptr);
      case 6:
        return func((const T6 *)ptr);
      case 7:
        return func((const T7 *)ptr);
      default:
        return dispatch<F, R, Ts...>(std::forward<F>(func), ptr, index - 8);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7, typename... Ts,
              typename = typename std::enable_if_t<(sizeof...(Ts) > 0)>>
    SPECULA_CPU_GPU R dispatch(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      case 5:
        return func((T5 *)ptr);
      case 6:
        return func((T6 *)ptr);
      case 7:
        return func((T7 *)ptr);
      default:
        return dispatch<F, R, Ts...>(std::forward<F>(func), ptr, index - 8);
      }
    }

    template <typename F, typename R, typename T>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_EQ(0, index);
      return func((const T *)ptr);
    }

    template <typename F, typename R, typename T> R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_EQ(0, index);
      return func((T *)ptr);
    }

    template <typename F, typename R, typename T0, typename T1>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 2);

      if (index == 0) {
        return func((const T0 *)ptr);
      } else {
        return func((const T1 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 2);

      if (index == 0) {
        return func((T0 *)ptr);
      } else {
        return func((T1 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 3);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      default:
        return func((const T2 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 3);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      default:
        return func((T2 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 4);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      default:
        return func((const T3 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 4);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      default:
        return func((T3 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 5);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      default:
        return func((const T4 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 5);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      default:
        return func((T4 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 6);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      default:
        return func((const T5 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 6);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      default:
        return func((T5 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 7);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      case 5:
        return func((const T5 *)ptr);
      default:
        return func((const T6 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 7);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      case 5:
        return func((T5 *)ptr);
      default:
        return func((T6 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 8);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      case 5:
        return func((const T5 *)ptr);
      case 6:
        return func((const T6 *)ptr);
      default:
        return func((const T7 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);
      DASSERT_LT(index, 8);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      case 5:
        return func((T5 *)ptr);
      case 6:
        return func((T6 *)ptr);
      default:
        return func((T7 *)ptr);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7, typename... Ts,
              typename = typename std::enable_if_t<(sizeof...(Ts) > 0)>>
    R dispatch_cpu(F &&func, const void *ptr, int index) {
      DASSERT_GE(index, 0);

      switch (index) {
      case 0:
        return func((const T0 *)ptr);
      case 1:
        return func((const T1 *)ptr);
      case 2:
        return func((const T2 *)ptr);
      case 3:
        return func((const T3 *)ptr);
      case 4:
        return func((const T4 *)ptr);
      case 5:
        return func((const T5 *)ptr);
      case 6:
        return func((const T6 *)ptr);
      case 7:
        return func((const T7 *)ptr);
      default:
        return dispatch_cpu<F, R, Ts...>(std::forward<F>(func), ptr, index - 8);
      }
    }

    template <typename F, typename R, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7, typename... Ts,
              typename = typename std::enable_if_t<(sizeof...(Ts) > 0)>>
    R dispatch_cpu(F &&func, void *ptr, int index) {
      DASSERT_GE(index, 0);

      switch (index) {
      case 0:
        return func((T0 *)ptr);
      case 1:
        return func((T1 *)ptr);
      case 2:
        return func((T2 *)ptr);
      case 3:
        return func((T3 *)ptr);
      case 4:
        return func((T4 *)ptr);
      case 5:
        return func((T5 *)ptr);
      case 6:
        return func((T6 *)ptr);
      case 7:
        return func((T7 *)ptr);
      default:
        return dispatch_cpu<F, R, Ts...>(std::forward<F>(func), ptr, index - 8);
      }
    }

    template <typename... Ts> struct IsSameType;

    template <> struct IsSameType<> {
      static constexpr bool value = true;
    };

    template <typename T> struct IsSameType<T> {
      static constexpr bool value = true;
    };

    template <typename T, typename U, typename... Ts> struct IsSameType<T, U, Ts...> {
      static constexpr bool value = (std::is_same_v<T, U> && IsSameType<U, Ts...>::value);
    };

    template <typename... Ts> struct SameType;
    template <typename T, typename... Ts> struct SameType<T, Ts...> {
      using type = T;
      static_assert(IsSameType<T, Ts...>::value, "Not all types in pack are the same");
    };

    template <typename F, typename... Ts> struct ReturnType {
      using type = typename SameType<typename std::invoke_result_t<F, Ts *>...>::type;
    };

    template <typename F, typename... Ts> struct ReturnTypeConst {
      using type = typename SameType<typename std::invoke_result_t<F, const Ts *>...>::type;
    };

  } // namespace detail

  template <typename... Ts> class TaggedPointer {
  public:
    using Types = TypePack<Ts...>;

    TaggedPointer() = default;

    template <typename T> SPECULA_CPU_GPU TaggedPointer(T *ptr) {
      uint64_t iptr = reinterpret_cast<uint64_t>(ptr);
      DASSERT_EQ(iptr & ptr_mask, iptr);
      constexpr unsigned int type = type_index<T>();
      bits = iptr | ((uint64_t)type << tag_shift);
    }

    SPECULA_CPU_GPU TaggedPointer(std::nullptr_t np) {}

    SPECULA_CPU_GPU TaggedPointer(const TaggedPointer &t) { bits = t.bits; }
    SPECULA_CPU_GPU TaggedPointer *operator=(const TaggedPointer &t) {
      bits = t.bits;
      return this;
    }

    template <typename T> SPECULA_CPU_GPU static constexpr unsigned int type_index() {
      using Tp = typename std::remove_cv_t<T>;
      if constexpr (std::is_same_v<Tp, std::nullptr_t>)
        return 0;
      else
        return 1 + specula::IndexOf<Tp, Types>::count;
    }

    SPECULA_CPU_GPU unsigned int tag() const { return ((bits & tag_mask) >> tag_shift); }

    template <typename T> SPECULA_CPU_GPU bool is() const { return tag() == type_index<T>(); }

    SPECULA_CPU_GPU static constexpr unsigned int max_tag() { return sizeof...(Ts); }
    SPECULA_CPU_GPU static constexpr unsigned int num_tags() { return max_tag() + 1; }

    SPECULA_CPU_GPU explicit operator bool() const { return (bits & ptr_mask) != 0; }

    SPECULA_CPU_GPU bool operator<(const TaggedPointer &tp) const { return bits < tp.bits; }

    template <typename T> SPECULA_CPU_GPU T *cast() {
      DASSERT(is<T>());
      return reinterpret_cast<T *>(ptr());
    }

    template <typename T> SPECULA_CPU_GPU const T *cast() const {
      DASSERT(is<T>());
      return reinterpret_cast<const T *>(ptr());
    }

    template <typename T> SPECULA_CPU_GPU T *cast_or_nullptr() {
      if (is<T>())
        return reinterpret_cast<T *>(ptr());
      else
        return nullptr;
    }

    template <typename T> SPECULA_CPU_GPU const T *cast_or_nullptr() const {
      if (is<T>())
        return reinterpret_cast<const T *>(ptr());
      else
        return nullptr;
    }

    SPECULA_CPU_GPU void *ptr() { return reinterpret_cast<void *>(bits & ptr_mask); }
    SPECULA_CPU_GPU const void *ptr() const { return reinterpret_cast<void *>(bits & ptr_mask); }

    template <typename F> SPECULA_CPU_GPU decltype(auto) dispatch(F &&func) {
      DASSERT(ptr());
      using R = typename detail::ReturnType<F, Ts...>::type;
      return detail::dispatch<F, R, Ts...>(func, ptr(), tag() - 1);
    }

    template <typename F> SPECULA_CPU_GPU decltype(auto) dispatch(F &&func) const {
      DASSERT(ptr());
      using R = typename detail::ReturnTypeConst<F, Ts...>::type;
      return detail::dispatch<F, R, Ts...>(func, ptr(), tag() - 1);
    }

    template <typename F> decltype(auto) dispatch_cpu(F &&func) {
      DASSERT(ptr());
      using R = typename detail::ReturnType<F, Ts...>::type;
      return detail::dispatch_cpu<F, R, Ts...>(func, ptr(), tag() - 1);
    }

    template <typename F> decltype(auto) dispatch_cpu(F &&func) const {
      DASSERT(ptr());
      using R = typename detail::ReturnTypeConst<F, Ts...>::type;
      return detail::dispatch_cpu<F, R, Ts...>(func, ptr(), tag() - 1);
    }

  private:
    static_assert(sizeof(uintptr_t) <= sizeof(uint64_t), "Expected pointer size to be <= 64 bits");

    static constexpr int tag_shift = 57;
    static constexpr int tag_bits = 64 - tag_shift;
    static constexpr uint64_t tag_mask = ((1ull << tag_bits) - 1) << tag_shift;
    static constexpr uint64_t ptr_mask = ~tag_mask;

    uint64_t bits = 0;
  };
} // namespace specula

#endif // INCLUDE_UTIL_TAGGED_POINTER_HPP_
