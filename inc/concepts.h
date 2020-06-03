#ifndef __CONCEPTS_H
#define __CONCEPTS_H

#include "traits.h"

#define requires_T(...) \
        , typename = std::enable_if_t<(__VA_ARGS__)>

namespace concepts {
  // https://en.cppreference.com/w/cpp/concepts/same_as
  template<typename T, typename U>
  using SameAs_t = std::enable_if_t<std::is_same_v<T, U> && std::is_same_v<U, T>>;

  template <typename T, typename U>
  using SameAs = traits::is_detected<SameAs_t, T, U>;

  // https://en.cppreference.com/w/cpp/concepts/convertible_to
  template<typename From, typename To>
  using ExplicitlyConvertibleTo_t = decltype(static_cast<To>(std::declval<From>()));

  template<typename From, typename To>
  using ConvertibleTo_t = std::enable_if_t<
    std::conjunction_v<
      std::is_convertible<From, To>, //implicitly convertible
      traits::is_detected<ExplicitlyConvertibleTo_t, From, To>>>;

  template <typename From, typename To>
  using ConvertibleTo = traits::is_detected<ConvertibleTo_t, From, To>;

  //https://en.cppreference.com/w/cpp/concepts/common_reference_with
  template<typename T, typename U>
  using CommonReferenceWith_t = std::enable_if_t<
    std::conjunction_v<
      SameAs<traits::common_reference_t<T, U>, traits::common_reference_t<U, T>>,
      ConvertibleTo<T, traits::common_reference_t<T, U>>,
      ConvertibleTo<U, traits::common_reference_t<T, U>>>>;

  template<typename T, typename U>
  using CommonReferenceWith = traits::is_detected<CommonReferenceWith_t, T, U>;

  //https://en.cppreference.com/w/cpp/concepts/equality_comparable
  namespace detail {
    template<typename T, typename U>
    using WeaklyEqualityComparableWith_t = std::enable_if_t<
      std::conjunction_v<
        std::is_convertible<decltype(std::declval<traits::as_cref_t<T>>() == std::declval<traits::as_cref_t<U>>()), bool>,
        std::is_convertible<decltype(std::declval<traits::as_cref_t<T>>() != std::declval<traits::as_cref_t<U>>()), bool>,
        std::is_convertible<decltype(std::declval<traits::as_cref_t<U>>() == std::declval<traits::as_cref_t<T>>()), bool>,
        std::is_convertible<decltype(std::declval<traits::as_cref_t<U>>() != std::declval<traits::as_cref_t<T>>()), bool>>>;
  }

  template <typename T, typename U>
  using WeaklyEqualityComparableWith = traits::is_detected<detail::WeaklyEqualityComparableWith_t, T, U>;

  template <typename T>
  using EqualityComparable = WeaklyEqualityComparableWith<T, T>;

  template<typename T, typename U>
  using EqualityComparableWith_t = std::enable_if_t<
    std::conjunction_v<
      EqualityComparable<T>,
      EqualityComparable<U>,
      WeaklyEqualityComparableWith<T, U>,
      CommonReferenceWith<traits::as_cref_t<T>, traits::as_cref_t<U>>,
      EqualityComparable<traits::common_reference_t<traits::as_cref_t<T>, traits::as_cref_t<U>>>>>;

  template<typename T, typename U>
  using EqualityComparableWith = traits::is_detected<EqualityComparableWith_t, T, U>;
}

#endif
