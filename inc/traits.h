#pragma once

#ifndef __TRAITS_H
#define __TRAITS_H

#include <type_traits>

namespace traits {

  template<typename T>
  struct as_cref {
    using type = T const&;
  };
  template<typename T>
  struct as_cref<T&> {
    using type = T const&;
  };
  template<typename T>
  struct as_cref<T&&> {
    using type = T const&;
  };
  template<>
  struct as_cref<void> {
    using type = void;
  };
  template<>
  struct as_cref<void const> {
    using type = void const;
  };

  template<typename T>
  using as_cref_t = typename as_cref<T>::type;

  /**
   * \brief Takes an arbitrary enumerator and returns its value as a compile-time constant
   * \tparam E type of enumerator
   * \param enumerator
   * \return the value of enumerator
   */
  template <typename E>
  constexpr auto toUType(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
  }

  //
  //CXX 20
  //

  // https://en.cppreference.com/w/cpp/types/remove_cvref
  template<typename T>
  struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
  };

  template<typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;

  // common_reference
  // very very simplified implementation - just for comparable check on const lvalue references
  // https://en.cppreference.com/w/cpp/types/common_reference
  // https://ericniebler.github.io/std/wg21/D0022.html
  template<typename... Ts>
  struct common_reference {
  };

  namespace detail {
    template<typename T, typename U, typename = void>
    struct common_reference2 {
    };

    template<typename T, typename U>
    struct common_reference2<T, U,
      std::void_t<
      std::enable_if_t<
      std::is_reference_v<T>&& std::is_reference_v<U>>,
      std::common_type_t<remove_cvref_t<T>, remove_cvref_t<U>>>> {
      using type = as_cref_t<std::common_type_t<remove_cvref_t<T>, remove_cvref_t<U>>>;
    };
  }

  template<typename T>
  struct common_reference<T> {
    using type = T;
  };

  template<typename T, typename U>
  struct common_reference<T, U> : detail::common_reference2<T, U> {
  };

  template<typename T, typename U>
  using common_reference_t = typename common_reference<T, U>::type;

  //
  //  EXPERIMENTAL V2 type_traits; part of GCC/Clang, lack of support in MSVC
  //
  struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(const nonesuch&) = delete;
    void operator=(const nonesuch&) = delete;
    nonesuch(nonesuch&&) = delete;
    void operator=(nonesuch&&) = delete;
  };

  // https://people.eecs.berkeley.edu/~brock/blog/detection_idiom.php
  // https://blog.tartanllama.xyz/detection-idiom/
  // https://en.cppreference.com/w/cpp/experimental/is_detected
  namespace detail {
    template <class Default, class AlwaysVoid, template<class...> class Op, class... Args>
    struct detector {
      using value_t = std::false_type;
      using type = Default;
    };

    template <class Default, template<class...> class Op, class... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
      using value_t = std::true_type;
      using type = Op<Args...>;
    };
  }

  template <template<class...> class Op, class... Args>
  using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

  template<template<typename...> class Op, typename... Args>
  inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

  template <template<class...> class Op, class... Args>
  using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

  template <class Default, template<class...> class Op, class... Args>
  using detected_or = detail::detector<Default, void, Op, Args...>;

  template<typename Default, template<typename...> class Op, typename... Args>
  using detected_or_t = typename detected_or<Default, Op, Args...>::type;

  template<typename Expected, template<typename...> class Op, typename... Args>
  using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

  template<typename Expected, template<typename...> class Op, typename... Args>
  inline constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

  template<typename To, template<typename...> class Op, typename... Args>
  using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

  template<typename To, template<typename...> class Op, typename... Args>
  inline constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;
}

#endif
