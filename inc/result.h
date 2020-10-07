#pragma once

#ifndef RESULT_H
#define RESULT_H

#include "concepts.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>

namespace result {
  namespace option_type {
    /**
     * \brief The success value wrapper
     * \tparam T type of success value
     */
    template<typename T>
    struct Ok final {
      using type = T;

      template<
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<T, Args&&...>>>
      constexpr explicit Ok(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        : value(std::forward<Args>(args)...) {
      }

      constexpr Ok(const Ok&) = default;
      constexpr Ok(Ok&&) = default;
      constexpr Ok& operator=(const Ok&) = default;
      constexpr Ok& operator=(Ok&&) = default;

      ~Ok() = default;

      T value;
    };

    /**
     * \brief The success value wrapper specialized for void type
     */
    template<>
    struct Ok<void> final {
      using type = void;
    };

    /**
     * \brief The error value wrapper
     * \tparam E type of error value
     * \note E type cannot be void type
     */
    template<typename E>
    struct Err final {
      static_assert(!std::is_void_v<E>, "void as error type is not allowed");

      using type = E;

      template<
        typename... Args,
        typename = std::enable_if_t<std::is_constructible_v<E, Args&&...>>>
      constexpr explicit Err(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<E, Args&&...>)
        : value(std::forward<Args>(args)...) {
      }

      constexpr Err(const Err&) = default;
      constexpr Err(Err&&) = default;
      constexpr Err& operator=(const Err&) = default;
      constexpr Err& operator=(Err&&) = default;

      ~Err() = default;

      E value;
    };
  }

  /**
   * \brief Creates success value wrapper with the given success value
   * \tparam T type of success value
   * \param val success value
   * \return success value wrapper
   */
  template<typename T>
  constexpr option_type::Ok<std::decay_t<T>> Ok(T&& val)
  noexcept(noexcept(option_type::Ok<std::decay_t<T>>(std::forward<T>(val)))) {
    return option_type::Ok<std::decay_t<T>>(std::forward<T>(val));
  }

  template<
    typename T,
    typename Arg1,
    typename Arg2,
    typename... Args>
  constexpr option_type::Ok<T> Ok(Arg1&& arg1, Arg2&& arg2, Args&&... args)
  noexcept(noexcept(option_type::Ok<T>(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...))) {
    return option_type::Ok<T>(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...);
  }

  /**
   * \brief Creates success value wrapper specialized for void type
   * \return success value wrapper
   */
  constexpr option_type::Ok<void> Ok() noexcept {
    return {};
  }

  /**
   * \brief Creates error value wrapper with the given error value
   * \tparam E type of error value
   * \param val error value
   * \return error value wrapper
   */
  template<typename E>
  constexpr option_type::Err<std::decay_t<E>> Err(E&& val)
  noexcept(noexcept(option_type::Err<std::decay_t<E>>(std::forward<E>(val)))) {
    return option_type::Err<std::decay_t<E>>(std::forward<E>(val));
  }

  template<
    typename E,
    typename Arg1,
    typename Arg2,
    typename... Args>
  constexpr option_type::Err<E> Err(Arg1&& arg1, Arg2&& arg2, Args&&... args)
  noexcept(noexcept(option_type::Err<E>(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...))) {
    return option_type::Err<E>(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...);
  }

  /**
   * \brief Result represents either success (Ok) or failure (Err)
   * \tparam T type of success value
   * \tparam E type of failure value
   * \remark inspired by Rust's Result type:
   *   https://doc.rust-lang.org/std/result/enum.Result.html
   *   https://doc.rust-lang.org/std/result/index.html
   *   https://github.com/oktal/result
   */
  template<typename T, typename E>
  class Result;

  /**
   * \brief Checks whether R is a Result type
   * \tparam R a type to check
   */
  template<typename R>
  struct is_result : traits::is_template<R, Result> {
  };

  /**
   * \brief Helper variable template to check whether R is a Result type
   * \tparam R a type to check
   */
  template<typename R>
  inline constexpr bool is_result_v = is_result<R>::value;

  namespace detail {
    //
    //  CXX20
    //

    /**
     * \brief Creates a T object initialized with arguments args... at given address p
     * \tparam T type of object to create
     * \tparam Args types of arguments used for initialization
     * \param p pointer to the uninitialized storage on which a T object will be constructed
     * \param args arguments used for initialization
     * \return p
     * \note function will be a part of C++20 STL
     *   https://en.cppreference.com/w/cpp/memory/construct_at
     */
    template<typename T, typename... Args>
    constexpr auto construct_at(T* p, Args&&... args)
    noexcept(noexcept(::new(static_cast<void*>(nullptr)) T(std::declval<Args>()...))) ->
    decltype(::new(static_cast<void*>(nullptr)) T(std::declval<Args>()...)) {
      return ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
    }

    template<typename R>
    struct result_ok_type {
      using type = std::decay_t<R>;
    };

    template<typename T, typename E>
    struct result_ok_type<Result<T, E>> {
      using type = T;
    };

    template<typename R>
    using result_ok_type_t = typename result_ok_type<R>::type;

    template<typename R>
    struct result_err_type {
      using type = R;
    };

    template<typename T, typename E>
    struct result_err_type<Result<T, E>> {
      using type = std::remove_reference_t<E>;
    };

    template<typename R>
    using result_err_type_t = typename result_err_type<R>::type;

    struct ok_tag_t {
      constexpr explicit ok_tag_t() noexcept = default;
    };
    inline constexpr ok_tag_t ok_tag{};

    struct err_tag_t {
      constexpr explicit err_tag_t() noexcept = default;
    };
    inline constexpr err_tag_t err_tag{};

    struct default_ctor_tag_t {
      constexpr explicit default_ctor_tag_t() noexcept = default;
    };
    inline constexpr default_ctor_tag_t default_ctor_tag{};

    template<typename T,
      bool Enable = std::is_default_constructible_v<T> || std::is_void_v<T>>
    struct default_ctor_base {
      constexpr default_ctor_base() noexcept = default;
      constexpr default_ctor_base(const default_ctor_base&) noexcept = default;
      constexpr default_ctor_base(default_ctor_base&&) noexcept = default;
      default_ctor_base& operator=(const default_ctor_base&) noexcept = default;
      default_ctor_base& operator=(default_ctor_base&&) noexcept = default;
      ~default_ctor_base() noexcept = default;

      constexpr explicit default_ctor_base(default_ctor_tag_t) noexcept {
      }
    };

    template<typename T>
    struct default_ctor_base<T, false> {
      constexpr default_ctor_base() noexcept = delete;
      constexpr default_ctor_base(const default_ctor_base&) noexcept = default;
      constexpr default_ctor_base(default_ctor_base&&) noexcept = default;
      default_ctor_base& operator=(const default_ctor_base&) noexcept = default;
      default_ctor_base& operator=(default_ctor_base&&) noexcept = default;
      ~default_ctor_base() noexcept = default;

      constexpr explicit default_ctor_base(default_ctor_tag_t) noexcept {
      }
    };

    template<typename T, typename E>
    struct Storage final {
      using storage_type = std::aligned_union_t<sizeof(T), T, E>;

      constexpr Storage() : storage() {}

      template<typename U = T,
        typename = std::enable_if_t<std::is_same_v<T, U> && std::is_move_constructible_v<U>>>
      void construct(option_type::Ok<U>&& ok) noexcept(noexcept(detail::construct_at(static_cast<U*>(nullptr), std::move(ok.value)))) {
        detail::construct_at(reinterpret_cast<U*>(&storage), std::move(ok.value));
        initialized = true;
      }

      template<typename U = T,
        typename = std::enable_if_t<std::is_same_v<T, U> && std::is_copy_constructible_v<U>>>
      void construct(const option_type::Ok<U>& ok) noexcept(noexcept(detail::construct_at(static_cast<U*>(nullptr), ok.value))) {
        detail::construct_at(reinterpret_cast<U*>(&storage), ok.value);
        initialized = true;
      }

      void construct(option_type::Err<E>&& err) noexcept(noexcept(detail::construct_at(static_cast<E*>(nullptr), std::move(err.value)))) {
        detail::construct_at(reinterpret_cast<E*>(&storage), std::move(err.value));
        initialized = true;
      }

      void construct(const option_type::Err<E>& err) noexcept(noexcept(detail::construct_at(static_cast<E*>(nullptr), err.value))) {
        detail::construct_at(reinterpret_cast<E*>(&storage), err.value);
        initialized = true;
      }

      template<typename U, typename CleanU = std::decay_t<U>>
      void raw_construct(U&& val) noexcept(noexcept(detail::construct_at(static_cast<CleanU*>(nullptr), std::forward<U>(val)))) {
        detail::construct_at(reinterpret_cast<CleanU*>(&storage), std::forward<U>(val));
        initialized = true;
      }

      template<typename U>
      [[nodiscard]]
      const U& get() const& noexcept {
        return *reinterpret_cast<const U*>(&storage);
      }

      template<typename U>
      [[nodiscard]]
      U& get() & noexcept {
        return *reinterpret_cast<U*>(&storage);
      }

      template<typename U>
      [[nodiscard]]
      const U&& get() const&& noexcept {
        return std::move(*reinterpret_cast<const U*>(&storage));
      }

      template<typename U>
      [[nodiscard]]
      U&& get() && noexcept {
        return std::move(*reinterpret_cast<U*>(&storage));
      }

      void destroy(ok_tag_t) {
        if (initialized) {
          std::destroy_at(reinterpret_cast<T*>(&storage));
          initialized = false;
        }
      }

      void destroy(err_tag_t) {
        if (initialized) {
          std::destroy_at(reinterpret_cast<E*>(&storage));
          initialized = false;
        }
      }

      storage_type storage;
      bool initialized{ false };
    };

    template<typename E>
    struct Storage<void, E> final {
      using storage_type = std::aligned_storage_t<sizeof(E), alignof(E)>;

      constexpr Storage() : storage() {}

      void construct(option_type::Ok<void>&&) noexcept {
        initialized = true;
      }

      void construct(const option_type::Ok<void>&) noexcept {
        initialized = true;
      }

      void construct(option_type::Err<E>&& err) noexcept(noexcept(detail::construct_at(static_cast<E*>(nullptr), std::move(err.value)))) {
        detail::construct_at(reinterpret_cast<E*>(&storage), std::move(err.value));
        initialized = true;
      }

      void construct(const option_type::Err<E>& err) noexcept(noexcept(detail::construct_at(static_cast<E*>(nullptr), err.value))) {
        detail::construct_at(reinterpret_cast<E*>(&storage), err.value);
        initialized = true;
      }

      template<typename U, typename CleanU = std::decay_t<U>>
      void raw_construct(U&& val) noexcept(noexcept(detail::construct_at(static_cast<CleanU*>(nullptr), std::forward<U>(val)))) {
        if constexpr (!std::is_void_v<CleanU>) {
          detail::construct_at(reinterpret_cast<CleanU*>(&storage), std::forward<U>(val));
        }
        initialized = true;
      }

      template<typename U>
      [[nodiscard]]
      const U& get() const& noexcept {
        return *reinterpret_cast<const U*>(&storage);
      }

      template<typename U>
      [[nodiscard]]
      U& get() & noexcept {
        return *reinterpret_cast<U*>(&storage);
      }

      template<typename U>
      [[nodiscard]]
      const U&& get() const&& noexcept {
        return std::move(*reinterpret_cast<const U*>(&storage));
      }

      template<typename U>
      [[nodiscard]]
      U&& get() && noexcept {
        return std::move(*reinterpret_cast<U*>(&storage));
      }

      void destroy(ok_tag_t) noexcept {
        initialized = false;
      }

      void destroy(err_tag_t) {
        if (initialized) {
          std::destroy_at(reinterpret_cast<E*>(&storage));
          initialized = false;
        }
      }

      storage_type storage;
      bool initialized{ false };
    };

    template<typename T, typename E>
    struct Storage_Ctrl final {
      static void construct(const option_type::Ok<T>& ok, Storage<T, E>& dst) noexcept(noexcept(std::declval<Storage<T, E>&>().construct(ok))) {
        dst.construct(ok);
      }

      static void construct(option_type::Ok<T>&& ok, Storage<T, E>& dst) noexcept(noexcept(std::declval<Storage<T, E>&>().construct(std::move(ok)))) {
        dst.construct(std::move(ok));
      }

      static void construct(const option_type::Err<E>& err, Storage<T, E>& dst) noexcept(noexcept(std::declval<Storage<T, E>&>().construct(err))) {
        dst.construct(err);
      }

      static void construct(option_type::Err<E>&& err, Storage<T, E>& dst) noexcept(noexcept(std::declval<Storage<T, E>&>().construct(std::move(err)))) {
        dst.construct(std::move(err));
      }

      static void copy_construct(const Storage<T, E>& src, Storage<T, E>& dst, ok_tag_t) {
        dst.raw_construct(src.template get<T>());
      }

      static void move_construct(Storage<T, E>&& src, Storage<T, E>& dst, ok_tag_t tag) {
        dst.raw_construct(std::move(src.template get<T>()));
        src.destroy(tag);
      }

      template<typename DTag>
      static void copy_assignment(const Storage<T, E>& src, Storage<T, E>& dst, ok_tag_t, DTag dst_tag) {
        Storage<T, E> tmp;
        tmp.raw_construct(src.template get<T>());
        dst.destroy(dst_tag);
        dst.raw_construct(std::move(tmp.template get<T>()));
      }

      template<typename DTag>
      static void move_assignment(Storage<T, E>&& src, Storage<T, E>& dst, ok_tag_t, DTag dst_tag) {
        dst.destroy(dst_tag);
        dst.raw_construct(std::move(src.template get<T>()));
        src.destroy(ok_tag);
      }

      static void copy_construct(const Storage<T, E>& src, Storage<T, E>& dst, err_tag_t) {
        dst.raw_construct(src.template get<E>());
      }

      static void move_construct(Storage<T, E>&& src, Storage<T, E>& dst, err_tag_t tag) {
        dst.raw_construct(std::move(src.template get<E>()));
        src.destroy(tag);
      }

      template<typename DTag>
      static void copy_assignment(const Storage<T, E>& src, Storage<T, E>& dst, err_tag_t, DTag dst_tag) {
        Storage<T, E> tmp;
        tmp.raw_construct(src.template get<E>());
        dst.destroy(dst_tag);
        dst.raw_construct(std::move(tmp.template get<E>()));
      }

      template<typename DTag>
      static void move_assignment(Storage<T, E>&& src, Storage<T, E>& dst, err_tag_t, DTag dst_tag) {
        dst.destroy(dst_tag);
        dst.raw_construct(std::move(src.template get<E>()));
        src.destroy(err_tag);
      }

      template<typename Tag>
      static void destroy(Storage<T, E>& src, Tag tag) {
        src.destroy(tag);
      }
    };

    template<typename E>
    struct Storage_Ctrl<void, E> final {
      static void construct(const option_type::Ok<void>& ok, Storage<void, E>& dst) noexcept(noexcept(std::declval<Storage<void, E>&>().construct(ok))) {
        dst.construct(ok);
      }

      static void construct(option_type::Ok<void>&& ok, Storage<void, E>& dst) noexcept(noexcept(std::declval<Storage<void, E>&>().construct(std::move(ok)))) {
        dst.construct(ok);
      }

      static void construct(const option_type::Err<E>& err, Storage<void, E>& dst) noexcept(noexcept(std::declval<Storage<void, E>&>().construct(err))) {
        dst.construct(err);
      }

      static void construct(option_type::Err<E>&& err, Storage<void, E>& dst) noexcept(noexcept(std::declval<Storage<void, E>&>().construct(std::move(err)))) {
        dst.construct(std::move(err));
      }

      static void copy_construct(const Storage<void, E>& src, Storage<void, E>& dst, ok_tag_t) noexcept {
        dst.raw_construct(src.template get<void>());
      }

      static void move_construct(Storage<void, E>&& src, Storage<void, E>& dst, ok_tag_t) noexcept {
        dst.raw_construct(std::move(src.template get<void>()));
        src.destroy(ok_tag);
      }

      template<typename DTag>
      static void copy_assignment(const Storage<void, E>& src, Storage<void, E>& dst, ok_tag_t, DTag dst_tag) {
        dst.destroy(dst_tag);
      }

      template<typename DTag>
      static void move_assignment(Storage<void, E>&& src, Storage<void, E>& dst, ok_tag_t, DTag dst_tag) {
        dst.destroy(dst_tag);
      }

      static void copy_construct(const Storage<void, E>& src, Storage<void, E>& dst, err_tag_t) {
        dst.raw_construct(src.template get<E>());
      }

      static void move_construct(Storage<void, E>&& src, Storage<void, E>& dst, err_tag_t) {
        dst.raw_construct(std::move(src.template get<E>()));
        src.destroy(err_tag);
      }

      template<typename DTag>
      static void copy_assignment(const Storage<void, E>& src, Storage<void, E>& dst, err_tag_t, DTag dst_tag) {
        Storage<void, E> tmp;
        tmp.raw_construct(src.template get<E>());
        dst.destroy(dst_tag);
        dst.raw_construct(std::move(tmp.template get<E>()));
      }

      template<typename DTag>
      static void move_assignment(Storage<void, E>&& src, Storage<void, E>& dst, err_tag_t, DTag dst_tag) {
        dst.destroy(dst_tag);
        dst.raw_construct(std::move(src.template get<E>()));
        src.destroy(err_tag);
      }

      template<typename Tag>
      static void destroy(Storage<void, E>& src, Tag tag) {
        src.destroy(tag);
      }
    };
  }

  template<typename T, typename E>
  class Result final : private detail::default_ctor_base<T> {
    static_assert(!std::is_reference_v<T>, "T must not be a reference");
    static_assert(!std::is_reference_v<E>, "E must not be a reference");
    static_assert(!std::is_void_v<E>, "void as error type is not allowed");
    using ctor_base = detail::default_ctor_base<T>;

  public:
    using value_type = T;
    using error_type = E;
    using storage_type = detail::Storage<T, E>;
    using storage_ctrl = detail::Storage_Ctrl<T, E>;

    constexpr Result() = default;

    template<typename U = T,
      typename = std::enable_if_t<std::is_same_v<T, U> && !std::is_void_v<U> && std::is_copy_constructible_v<U>>>
    Result(const option_type::Ok<U>& ok) noexcept(noexcept(storage_ctrl::construct(ok, std::declval<storage_type&>())))
      : ctor_base(detail::default_ctor_tag) {
      storage_ctrl::construct(ok, storage);
    }

    template<typename U = T,
      typename = std::enable_if_t<std::is_same_v<T, U> && !std::is_void_v<U> && std::is_move_constructible_v<U>>>
    Result(option_type::Ok<U>&& ok) noexcept(noexcept(storage_ctrl::construct(std::move(ok), std::declval<storage_type&>())))
      : ctor_base(detail::default_ctor_tag) {
      storage_ctrl::construct(std::move(ok), storage);
    }

    template<typename U = T,
      typename = std::enable_if_t<std::is_same_v<T, U> && std::is_void_v<U>>>
    Result(const option_type::Ok<void>& ok) noexcept(noexcept(storage_ctrl::construct(ok, std::declval<storage_type&>())))
      : ctor_base(detail::default_ctor_tag) {
      storage_ctrl::construct(ok, storage);
    }

    template<typename U = T,
      typename = std::enable_if_t<std::is_same_v<T, U> && std::is_void_v<U>>>
    Result(option_type::Ok<void>&& ok) noexcept(noexcept(storage_ctrl::construct(std::move(ok), std::declval<storage_type&>())))
      : ctor_base(detail::default_ctor_tag) {
      storage_ctrl::construct(std::move(ok), storage);
    }


    Result(const option_type::Err<E>& err) noexcept(noexcept(storage_ctrl::construct(err, std::declval<storage_type&>())))
      : ctor_base(detail::default_ctor_tag), has_value(false) {
      storage_ctrl::construct(err, storage);
    }

    Result(option_type::Err<E>&& err) noexcept(noexcept(storage_ctrl::construct(std::move(err), std::declval<storage_type&>())))
      : ctor_base(detail::default_ctor_tag), has_value(false) {
      storage_ctrl::construct(std::move(err), storage);
    }

    Result(const Result& other) : ctor_base(detail::default_ctor_tag), has_value(other.has_value) {
      if (has_value) {
        storage_ctrl::copy_construct(other.storage, storage, detail::ok_tag);
      } else {
        storage_ctrl::copy_construct(other.storage, storage, detail::err_tag);
      }
    }

    Result(Result&& other) noexcept : ctor_base(detail::default_ctor_tag), has_value(std::move(other.has_value)) {
      if (has_value) {
        storage_ctrl::move_construct(std::move(other.storage), storage, detail::ok_tag);
      } else {
        storage_ctrl::move_construct(std::move(other.storage), storage, detail::err_tag);
      }
    }

    Result& operator=(const Result& other) {
      if (this != &other) {
        if (other.has_value) {
          if (has_value) {
            storage_ctrl::copy_assignment(
              other.storage,
              storage,
              detail::ok_tag,
              detail::ok_tag);
          } else {
            storage_ctrl::copy_assignment(
              other.storage,
              storage,
              detail::ok_tag,
              detail::err_tag);
          }
        } else {
          if (has_value) {
            storage_ctrl::copy_assignment(
              other.storage,
              storage,
              detail::err_tag,
              detail::ok_tag);
          } else {
            storage_ctrl::copy_assignment(
              other.storage,
              storage,
              detail::err_tag,
              detail::err_tag);
          }
        }
        has_value = other.has_value;
      }
      return *this;
    }

    Result& operator=(Result&& other) noexcept {
      if (this != &other) {
        if (other.has_value) {
          if (has_value) {
            storage_ctrl::move_assignment(
              std::move(other.storage),
              storage,
              detail::ok_tag,
              detail::ok_tag);
          } else {
            storage_ctrl::move_assignment(
              std::move(other.storage),
              storage,
              detail::ok_tag,
              detail::err_tag);
          }
        } else {
          if (has_value) {
            storage_ctrl::move_assignment(
              std::move(other.storage),
              storage,
              detail::err_tag,
              detail::ok_tag);
          } else {
            storage_ctrl::move_assignment(
              std::move(other.storage),
              storage,
              detail::err_tag,
              detail::err_tag);
          }
        }
        has_value = std::move(other.has_value);
      }
      return *this;
    }

    ~Result() {
      if (has_value) {
        storage_ctrl::destroy(storage, detail::ok_tag);
      } else {
        storage_ctrl::destroy(storage, detail::err_tag);
      }
    }

    /**
     * \brief Checks whether the result is Ok.
     * \return Returns true if the result is Ok.
     */
    [[nodiscard]]
    bool is_ok() const noexcept {
      return has_value;
    }

    /**
     * \brief Checks whether the result is Err.
     * \return Returns true if the result is Err.
     */
    [[nodiscard]]
    bool is_err() const noexcept {
      return !has_value;
    }

    /**
     * \brief Converts from Result<T, E> to std::optional<T>
     * \return std::optional<T>(), when Result is_ok; std::nullopt when Result is_err
     */
    [[nodiscard]]
    std::optional<T> ok() const {
      return is_ok() ? storage.template get<T>() : std::optional<T>();
    }

    /**
     * \brief Converts from Result<T, E> to std::optional<E>
     * \return std::optional<E>(), when Result is_err; std::nullopt when Result is_ok
     */
    [[nodiscard]]
    std::optional<E> err() const {
      return is_err() ? storage.template get<E>() : std::optional<E>();
    }

    /**
     * \brief Returns the contained Ok value. Panics if the value is an Err,
     *  with a panic message including the passed message, and the content of the Err
     * \param msg message
     * \return contained Ok value
     */
    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<T, U>)>
    const U& expect(const char* msg) const& {
      if (is_ok()) {
          return storage.template get<T>();
      }

      report_err<error_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<T, U>)>
    U& expect(const char* msg) & {
      if (is_ok()) {
        return storage.template get<T>();
      }

      report_err<error_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<T, U>)>
    const U&& expect(const char* msg) const&& {
      if (is_ok()) {
        return std::move(storage).template get<T>();
      }

      report_err<error_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<T, U>)>
    U&& expect(const char* msg) && {
      if (is_ok()) {
        return std::move(storage).template get<T>();
      }

      report_err<error_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    /**
     * \brief Returns the contained Err value. Panics if the value is an Ok,
     *  with a panic message including the passed message, and the content of the Ok
     * \param msg message
     * \return contained Err value
     */
    const E& expect_err(const char* msg) const& {
      if (is_err()) {
        return storage.template get<E>();
      }

      report_err<value_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    E& expect_err(const char* msg) & {
      if (is_err()) {
        return storage.template get<E>();
      }

      report_err<value_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    const E&& expect_err(const char* msg) const&& {
      if (is_err()) {
        return std::move(storage).template get<E>();
      }

      report_err<value_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    E&& expect_err(const char* msg) && {
      if (is_err()) {
        return std::move(storage).template get<E>();
      }

      report_err<value_type>(msg);
      std::exit(EXIT_FAILURE);
    }

    /**
     * \brief Maps a Result<T, E> to Result<U, E> by applying a function "op" to a contained Ok value, leaving an Err value untouched.
     * \tparam Func type of an operation; where Func(T) -> U
     * \param op operation
     * \return if the result is_ok, applies "op" on contained Ok value and returns Result<U, E>, leaving an Err value untouched.
     *   Otherwise, the Err value of result is returned in new Result<U, E>.
     * \remark This function can be used to compose the results of two functions.
     */
    template<typename U = T,
      typename Func
      requires_T(std::is_same_v<T, U> && !std::is_void_v<U> && std::is_invocable_v<Func, U>)>
    Result<std::invoke_result_t<Func, U>, E>
    map(Func&& op) const {
      if (is_ok()) {
        return Ok(std::invoke(std::forward<Func>(op), storage.template get<U>()));
      }
      return Err(storage.template get<E>());
    }

    /**
     * \brief Maps a Result<void, E> to Result<U, E> by applying a function op to a contained Ok value, leaving an Err value untouched.
     * \tparam Func type of an operation; where Func(void) -> U
     * \param op operation
     * \return if the result is_ok, applies "op" on contained Ok value and returns Result<U, E>, leaving an Err value untouched.
     *   Otherwise, the Err value of result is returned in new Result<U, E>.
     * \remark This function can be used to compose the results of two functions.
     */
    template<typename U = T,
      typename Func
      requires_T(std::is_same_v<T, U> && std::is_void_v<U> && std::is_invocable_v<Func>)>
    Result<std::invoke_result_t<Func>, E>
    map(Func&& op) const {
      if (is_ok()) {
        return Ok(std::invoke(std::forward<Func>(op)));
      }
      return Err(storage.template get<E>());
    }

    /**
     * \brief Maps a Result<T, E> to Result<T, F> by applying a function to a contained Err value, leaving an Ok value untouched.
     * \tparam Func type of an operation; where Func(E) -> F
     * \param op operation
     * \return If result is_err, applies "op" on contained Err value and returns Result<T, F>, leaving an Ok value untouched.
     *   Otherwise, the Ok value of result is returned in new Result<T, F>.
     * \remark This function can be used to pass through a successful result while handling an error.
     */
    template<typename Func>
    Result<T, std::invoke_result_t<Func, E>>
    map_err(Func&& op) const {
      if (is_err()) {
        return Err(std::invoke(std::forward<Func>(op), storage.template get<E>()));
      }
      return Ok(storage.template get<T>());
    }

    /**
     * \brief Applies a function to the contained value (if any), or returns the provided default (if not)
     * \tparam Func type of an operation; where Func(T) -> U
     * \param value default value
     * \param op operation
     * \return Applies a function to the contained value (if any), or returns the provided default (if not)
     * \remark Arguments passed to map_or are eagerly evaluated;
     *   if you are passing the result of a function call,
     *   it is recommended to use map_or_else, which is lazily evaluated
     */
    template<typename U = T,
      typename Func
      requires_T(std::is_same_v<T, U> && !std::is_void_v<U> && std::is_invocable_v<Func, U>)>
    std::enable_if_t<!std::is_void_v<std::invoke_result_t<Func, U>>, std::invoke_result_t<Func, U>>
    map_or(const traits::remove_cvref_t<std::invoke_result_t<Func, U>>& value, Func&& op) const {
      if (is_ok()) {
        return std::invoke(std::forward<Func>(op), storage.template get<U>());
      }
      return value;
    }

    /**
     * \brief Applies a function to the contained value (if any), or returns the provided default (if not)
     * \tparam Func type of an operation; where Func(void) -> U
     * \param value default value
     * \param op operation
     * \return Applies a function to the contained value (if any), or returns the provided default (if not)
     * \remark Arguments passed to map_or are eagerly evaluated;
     *   if you are passing the result of a function call,
     *   it is recommended to use map_or_else, which is lazily evaluated
     */
    template<typename U = T,
      typename Func
      requires_T(std::is_same_v<U, T> && std::is_void_v<U> && std::is_invocable_v<Func>)>
    std::enable_if_t<!std::is_void_v<std::invoke_result_t<Func>>, std::invoke_result_t<Func>>
    map_or(const traits::remove_cvref_t<std::invoke_result_t<Func>>& value, Func&& op) const {
      if (is_ok()) {
        return std::invoke(std::forward<Func>(op));
      }
      return value;
    }

    /**
     * \brief Maps a Result<T, E> to U by applying a function "op" to a contained Ok value, or a default function "dop" to a contained Err value.
     * \tparam Func type of an operation; where Func(T) -> U
     * \tparam DefaultFunc type of an default operation; when DefaultFunc(E) -> U
     * \param dop default operation
     * \param op operation
     * \return if result is_ok, returns a result of "op", taking result's Ok value; otherwise it returns a result of "dop", taking result's Err value
     * \remark This function can be used to unpack a successful result while handling an error.
     */
    template<typename U = T,
      typename Func,
      typename DefaultFunc
      requires_T(
        std::is_same_v<T, U> &&
        !std::is_void_v<U> &&
        std::is_invocable_v<Func, U> &&
        std::is_invocable_v<DefaultFunc, E>)>
    std::enable_if_t<std::is_convertible_v<std::invoke_result_t<DefaultFunc, E>, std::invoke_result_t<Func, U>>, std::invoke_result_t<Func, U>>
    map_or_else(DefaultFunc&& dop, Func&& op) const {
      if (is_ok()) {
        return std::invoke(std::forward<Func>(op), storage.template get<U>());
      }
      return std::invoke(std::forward<DefaultFunc>(dop), storage.template get<E>());
    }

    /**
     * \brief Maps a Result<void, E> to U by applying a function "op" to a contained Ok value, or a default function "dop" to a contained Err value.
     * \tparam Func type of an operation; where Func(void) -> U
     * \tparam DefaultFunc type of an default operation; when DefaultFunc(E) -> U
     * \param dop default operation
     * \param op operation
     * \return if result is_ok, returns a result of "op", taking result's Ok value; otherwise it returns a result of "dop", taking result's Err value
     * \remark This function can be used to unpack a successful result while handling an error.
     */
    template<typename U = T,
      typename Func,
      typename DefaultFunc
      requires_T(
        std::is_same_v<U, T> &&
        std::is_void_v<U> &&
        std::is_invocable_v<Func> &&
        std::is_invocable_v<DefaultFunc, E>)>
    std::enable_if_t<std::is_same_v<std::invoke_result_t<DefaultFunc, E>, std::invoke_result_t<Func>>, std::invoke_result_t<Func>>
    map_or_else(DefaultFunc&& dop, Func&& op) const {
      if (is_ok()) {
        return std::invoke(std::forward<Func>(op));
      }
      return std::invoke(std::forward<DefaultFunc>(dop), storage.template get<E>());
    }

    /**
     * \brief Variant of Rust's Result::and operation
     * \tparam U type of success value
     * \param res result
     * \return Returns res if the result is Ok, otherwise returns the Err value of self
     */
    template<typename U>
    [[nodiscard]]
    Result<U, E> operator&&(const Result<U, E>& res) const {
      if (is_ok()) {
        return res;
      }
      return Err(storage.template get<E>());
    }

    /**
     * \brief Calls op if the result is Ok, otherwise returns the Err value of self.
     * \tparam Func type of operation; where Func(T) -> Result<U, E>
     * \param op operation
     * \return if the result is Ok returns op result, otherwise returns the Err value of self.
     * \remark This function can be used for control flow based on result values.
     */
    template<typename U = T,
      typename Func
      requires_T(std::is_same_v<T, U> && !std::is_void_v<U> && std::is_invocable_v<Func, U>)>
    std::enable_if_t<is_result_v<std::invoke_result_t<Func, U>> && std::is_same_v<detail::result_err_type_t<std::invoke_result_t<Func, U>>, E>, std::invoke_result_t<Func, U>>
    and_then(Func&& op) const {
      if (is_ok()) {
        return std::invoke(std::forward<Func>(op), storage.template get<U>());
      }
      return Err(storage.template get<E>());
    }

    /**
     * \brief Calls op if the result is Ok, otherwise returns the Err value of self.
     * \tparam Func type of operation; where Func(void) -> Result<U, E>
     * \param op operation
     * \return if the result is Ok returns op result, otherwise returns the Err value of self.
     * \remark This function can be used for control flow based on result values.
     */
    template<typename U = T,
      typename Func
      requires_T(std::is_same_v<U, T> && std::is_void_v<U> && std::is_invocable_v<Func>)>
    std::enable_if_t<is_result_v<std::invoke_result_t<Func>> && std::is_same_v<detail::result_err_type_t<std::invoke_result_t<Func>>, E>, std::invoke_result_t<Func>>
    and_then(Func&& op) const {
      if (is_ok()) {
        return std::invoke(std::forward<Func>(op));
      }
      return Err(storage.template get<E>());
    }

    /**
     * \brief Variant of Rust's Result::or operation
     * \tparam F type of error value
     * \param res result
     * \return Returns res if the result is Err, otherwise returns the Ok value of self.
     */
    template<typename F>
    [[nodiscard]]
    Result<T, F> operator||(const Result<T, F>& res) const {
      if (is_err()) {
        return res;
      }
      return Ok(storage.template get<T>());
    }

    /**
     * \brief Calls op if the result is Err, otherwise returns the Ok value of self.
     * \tparam Func type of operation; where Func(E) -> Result<T, F>
     * \tparam Ret type of return value; Result<T, F>
     * \param op operation
     * \return if the result is Err returns op result, otherwise returns the Ok value of self.
     * \remark This function can be used for control flow based on result values.
    */
    template<typename Func,
      typename Ret = std::invoke_result_t<Func, E>
      requires_T(is_result_v<Ret> && std::is_same_v<detail::result_ok_type_t<Ret>, T>)>
    Ret or_else(Func&& op) const {
      if (is_err()) {
        return std::invoke(std::forward<Func>(op), storage.template get<E>());
      }
      return Ok(storage.template get<T>());
    }

    /**
     * \brief Returns the contained Ok value
     * \return when Result is_ok, returns the contained Ok value;
     *  when result is_err, panics with a panic message provided by the Err's value
     * \remark Because this function may panic, its use is generally discouraged.
     *  Instead, prefer to use pattern matching and handle the Err case explicitly,
     *  or call unwrap_or, unwrap_or_else, or unwrap_or_default.
     */
    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<U, T>)>
    [[nodiscard]]
    const U& unwrap() const& {
      if (is_ok()) {
        return storage.template get<U>();
      }

      report_err<error_type>("Attempting to unwrap an Err Result");
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<U, T>)>
    [[nodiscard]]
    U& unwrap() & {
      if (is_ok()) {
        return storage.template get<U>();
      }

      report_err<error_type>("Attempting to unwrap an Err Result");
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<U, T>)>
    [[nodiscard]]
    const U&& unwrap() const&& {
      if (is_ok()) {
        return std::move(storage).template get<U>();
      }

      report_err<error_type>("Attempting to unwrap an Err Result");
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(!std::is_void_v<U> && std::is_same_v<U, T>)>
    [[nodiscard]]
    U&& unwrap() && {
      if (is_ok()) {
        return std::move(storage).template get<T>();
      }

      report_err<error_type>("Attempting to unwrap an Err Result");
      std::exit(EXIT_FAILURE);
    }

    template<typename U = T
      requires_T(std::is_void_v<U> && std::is_same_v<U, T>)>
    void unwrap() & {
      if (is_err()) {
        report_err<error_type>("Attempting to unwrap an Err Result");
        std::exit(EXIT_FAILURE);
      }
    }

    template<typename U = T
      requires_T(std::is_void_v<U>&& std::is_same_v<U, T>)>
    void unwrap() const& {
      if (is_err()) {
        report_err<error_type>("Attempting to unwrap an Err Result");
        std::exit(EXIT_FAILURE);
      }
    }

    template<typename U = T
      requires_T(std::is_void_v<U>&& std::is_same_v<U, T>)>
    void unwrap() && {
      if (is_err()) {
        report_err<error_type>("Attempting to unwrap an Err Result");
        std::exit(EXIT_FAILURE);
      }
    }

    template<typename U = T
      requires_T(std::is_void_v<U>&& std::is_same_v<U, T>)>
    void unwrap() const&& {
      if (is_err()) {
        report_err<error_type>("Attempting to unwrap an Err Result");
        std::exit(EXIT_FAILURE);
      }
    }

    /**
     * \brief Returns the contained Err value
     * \return when Result is_err, returns the contained Err value;
     *  when result is_ok, panics with a custom panic message provided by the Ok's value
     */
    [[nodiscard]]
    const E& unwrap_err() const& {
      if (is_err()) {
        return storage.template get<E>();
      }

      report_err<value_type>("Attempting to unwrap_err an Ok Result");
      std::exit(EXIT_FAILURE);
    }

    [[nodiscard]]
    E& unwrap_err() & {
      if (is_err()) {
        return storage.template get<E>();
      }

      report_err<value_type>("Attempting to unwrap_err an Ok Result");
      std::exit(EXIT_FAILURE);
    }

    [[nodiscard]]
    const E&& unwrap_err() const&& {
      if (is_err()) {
        return std::move(storage).template get<E>();
      }

      report_err<value_type>("Attempting to unwrap_err an Ok Result");
      std::exit(EXIT_FAILURE);
    }

    [[nodiscard]]
    E&& unwrap_err() && {
      if (is_err()) {
        return std::move(storage).template get<E>();
      }

      report_err<value_type>("Attempting to unwrap_err an Ok Result");
      std::exit(EXIT_FAILURE);
    }

    /**
     * \brief Returns the contained Ok value or a provided default
     * \tparam U the type of provided default value
     * \param value default value
     * \return if Result is_ok, returns success value; otherwise returns default value
     * \remark  Arguments passed to unwrap_or are eagerly evaluated;
     *  if you are passing the result of a function call,
     *  it is recommended to use unwrap_or_else, which is lazily evaluated.
     */
    template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>&& std::is_convertible_v<U, T>>>
    [[nodiscard]]
    T unwrap_or(const U& value) const {
      if (is_ok()) {
        return storage.template get<T>();
      }
      return value;
    }

  /**
   * \brief Returns the contained Ok value or a default value of the success type
   * \return if Result is_ok, returns the contained value, otherwise if it is_err,
   *   returns the default value of the success type.
   */
  template<typename U = T
    requires_T(
      !std::is_void_v<U> &&
      std::is_same_v<T, U> &&
      std::is_default_constructible_v<U> &&
      std::is_copy_constructible_v<T>)>
    [[nodiscard]]
    T unwrap_or_default() const {
      if (is_ok()) {
        return storage.template get<T>();
      }
      return {};
    }

    /**
     * \brief Returns the contained Ok value or computes it from a closure
     * \tparam Func type of operation; where Func(E) -> T
     * \param op operation
     * \return if Result is_ok, returns the contained Ok value or computes it from an operation
     */
    template<typename Func
      requires_T(std::is_same_v<std::invoke_result_t<Func, E>, T>)>
    [[nodiscard]]
    T unwrap_or_else(Func&& op) const {
      if (is_err()) {
        return std::invoke(std::forward<Func>(op), storage.template get<E>());
      }
      return storage.template get<T>();
    }

    /**
     * \brief Checks Ok success value whether contains the given value
     * \tparam U type of the given value
     * \param value given success value to check
     * \return Returns true if the result is an Ok value containing the given value
     * \remark function is defined for non void success type
     */
    template<typename U = T
      requires_T(
        !std::is_void_v<U> &&
        concepts::EqualityComparableWith<T, U>::value)>
    [[nodiscard]]
    bool contains(const U& value) const noexcept {
      if (is_ok()) {
        return storage.template get<T>() == value;
      }
      return false;
    }

    /**
     * \brief Checks Err error value whether contains the given value
     * \tparam F type of the given value
     * \param value given error value to check
     * \return Returns true if the result is an Err value containing the given value
     */
    template<typename F = E
      requires_T(concepts::EqualityComparableWith<E, F>::value)>
    [[nodiscard]]
    bool contains_err(const F& value) const noexcept {
      if (is_err()) {
        return storage.template get<E>() == value;
      }
      return false;
    }

    /**
     * \brief Transposes a Result of an Option into an Option of a Result
     * \return Ok(std::nullopt) will be mapped to std::nullopt;
     *   Ok(std::optional<T>) and Err(E) will be mapped to std::optional(Ok(T)) and std::optional(Err(E)).
     */
    template<typename U = T
      requires_T(std::is_same_v<U, T> && traits::is_template_v<T, std::optional>)>
    [[nodiscard]]
    std::optional<Result<typename U::value_type, E>> transpose() const {
      if (is_ok()) {
        auto optional_value = storage.template get<T>();
        if (optional_value) {
          return Ok(optional_value.value());
        }
        return {};
      }
      return Err(storage.template get<E>());
    }

  private:
    template<typename U>
    void report_err(const char* msg) const {
      if constexpr (!std::is_void_v<U>) {
        if constexpr (std::is_enum_v<U>) {
          std::cerr << msg << ": " << traits::toUType(storage.template get<U>()) << std::endl;
        } else {
          std::cerr << msg << ": " << storage.template get<U>() << std::endl;
        }
      } else {
        std::cerr << msg << std::endl;
      }
    }

    bool has_value{ true };
    storage_type storage;
  };

  template<typename T1, typename E1, typename T2, typename E2
    requires_T(
      concepts::EqualityComparableWith<T1, T2>::value &&
      concepts::EqualityComparableWith<E1, E2>::value)>
  bool operator==(const Result<T1, E1>& lhs, const Result<T2, E2>& rhs) noexcept {
    return lhs.is_ok() != rhs.is_ok() ?
      false :
      !lhs.is_ok() ?
        lhs.unwrap_err() == rhs.unwrap_err() :
        lhs.unwrap() == rhs.unwrap();
  }

  template<typename E1, typename E2
    requires_T(concepts::EqualityComparableWith<E1, E2>::value)>
  bool operator==(const Result<void, E1>& lhs, const Result<void, E2>& rhs) noexcept {
    return lhs.is_ok() != rhs.is_ok() ?
      false :
      !lhs.is_ok() ?
        lhs.unwrap_err() == rhs.unwrap_err() :
        true;
  }

  template<typename T1, typename E1, typename T2, typename E2
    requires_T(
      concepts::EqualityComparableWith<T1, T2>::value &&
      concepts::EqualityComparableWith<E1, E2>::value)>
  bool operator!=(const Result<T1, E1>& lhs, const Result<T2, E2>& rhs) noexcept {
  return lhs.is_ok() != rhs.is_ok() ?
    true :
    !lhs.is_ok() ?
      lhs.unwrap_err() != rhs.unwrap_err() :
      lhs.unwrap() != rhs.unwrap();
}

  template<typename E1, typename E2
    requires_T(concepts::EqualityComparableWith<E1, E2>::value)>
  bool operator!=(const Result<void, E1>& lhs, const Result<void, E2>& rhs) noexcept {
    return lhs.is_ok() != rhs.is_ok() ?
      true :
      !lhs.is_ok() ?
        lhs.unwrap_err() != rhs.unwrap_err() :
        true;
  }

  template<typename T1, typename E1, typename T2
    requires_T(concepts::EqualityComparableWith<T1, T2>::value)>
  bool operator==(const Result<T1, E1>& r, const option_type::Ok<T2>& ok) noexcept {
    return r.is_ok() ? r.unwrap() == ok.value : false;
  }

  template<typename T1, typename E1, typename T2>
  bool operator==(const option_type::Ok<T2>& ok, const Result<T1, E1>& r) noexcept {
    return r == ok;
  }

  template<typename T1, typename E1, typename T2
    requires_T(concepts::EqualityComparableWith<T1, T2>::value)>
  bool operator!=(const Result<T1, E1>& r, const option_type::Ok<T2>& ok) noexcept {
    return r.is_ok() ? r.unwrap() != ok.value : true;
  }

  template<typename T1, typename E1, typename T2>
  bool operator!=(const option_type::Ok<T2>& ok, const Result<T1, E1>& r) noexcept {
    return r != ok;
  }

  template<typename E>
  bool operator==(const Result<void, E>& r, const option_type::Ok<void>&) noexcept {
    return r.is_ok();
  }

  template<typename E>
  bool operator==(const option_type::Ok<void>&, const Result<void, E>& r) noexcept {
    return r.is_ok();
  }

  template<typename E>
  bool operator!=(const Result<void, E>& r, const option_type::Ok<void>&) noexcept {
    return !r.is_ok();
  }

  template<typename E>
  bool operator!=(const option_type::Ok<void>&, const Result<void, E>& r) noexcept {
    return !r.is_ok();
  }

  template<typename T1, typename E1, typename E2
    requires_T(concepts::EqualityComparableWith<E1, E2>::value)>
  bool operator==(const Result<T1, E1>& r, const option_type::Err<E2>& err) noexcept {
    return r.is_err() ? r.unwrap_err() == err.value : false;
  }

  template<typename T1, typename E1, typename E2>
  bool operator==(const option_type::Err<E2>& err, const Result<T1, E1>& r) noexcept {
    return r == err;
  }

  template<typename T1, typename E1, typename E2
    requires_T(concepts::EqualityComparableWith<E1, E2>::value)>
  bool operator!=(const Result<T1, E1>& r, const option_type::Err<E2>& err) noexcept {
    return r.is_err() ? r.unwrap_err() != err.value : true;
  }

  template<typename T1, typename E1, typename E2>
  bool operator!=(const option_type::Err<E2>& err, const Result<T1, E1>& r) noexcept {
    return r != err;
  }
}

#endif
