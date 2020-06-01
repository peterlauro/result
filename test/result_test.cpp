#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "result.h"

#include <numeric>
#include <string>
#include <vector>

using namespace std::literals::string_literals;

enum class ErrorCode {
  ERROR_ONE,
  ERROR_TWO,
  ERROR_THREE
};

using container_type = std::vector<uint32_t>;
using size_type = container_type::size_type;
using value_type = container_type::value_type;
using result_type = result::Result<container_type, std::string>;

auto generate_container = [](value_type start, value_type stop) -> result_type {
  if constexpr (std::is_integral_v<value_type>) {
    if (start <= stop) {
      container_type container(static_cast<size_type>(stop) - start + 1U);
      std::iota(std::begin(container), std::end(container), static_cast<value_type>(start));
      return result::Ok(container);
    }
    return result::Err("stop is smaller than start"s);
  } else {
    return result::Err("value type is not integral"s);
  }
};

auto sum_container = [](container_type container)->value_type {
  return std::reduce(std::begin(container), std::end(container));
};

auto stringify = [](auto x) {
  std::stringstream ss;
  ss << "error code: " << x;
  return ss.str();
};

auto string_length = [](const auto& str) { return str.length(); };

auto sq = [](uint32_t x) -> result::Result<uint32_t, uint32_t> { return result::Ok(x * x); };
auto error = [](uint32_t x) -> result::Result<uint32_t, uint32_t> { return result::Err(x); };

TEST_SUITE("ResultTests") {

  TEST_CASE("IsOkTest") {
    const result::Result<double, ErrorCode> x = result::Ok(3.14);
    REQUIRE(x.is_ok());
    REQUIRE_FALSE(x.is_err());

    const result::Result<double, ErrorCode> y{};
    REQUIRE(y.is_ok());
    REQUIRE_FALSE(y.is_err());

    const result::Result<void, ErrorCode> z = result::Ok();
    REQUIRE(z.is_ok());
    REQUIRE_FALSE(z.is_err());

    const result::Result<void, ErrorCode> i{};
    REQUIRE(i.is_ok());
    REQUIRE_FALSE(i.is_err());

    result::Result<double, ErrorCode> j = result::Ok<double>(3);
    REQUIRE(x.is_ok());
    REQUIRE_FALSE(x.is_err());
  }

  TEST_CASE("IsErrTest") {
    const result::Result<double, ErrorCode> x = result::Err(ErrorCode::ERROR_ONE);
    REQUIRE(x.is_err());
    REQUIRE_FALSE(x.is_ok());
  }

  TEST_CASE("OkTest") {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    const auto ox = x.ok();
    REQUIRE(ox.has_value());
    REQUIRE_EQ(ox.value(), 2U);

    const result::Result<uint32_t, std::string> y = result::Err("Nothing here"s);
    const auto oy = y.ok();
    REQUIRE_FALSE(oy.has_value());
  }

  TEST_CASE("ErrTest") {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    const auto ox = x.err();
    REQUIRE_FALSE(ox.has_value());

    const result::Result<uint32_t, std::string> y = result::Err("Nothing here"s);
    const auto oy = y.err();
    REQUIRE(oy.has_value());
    REQUIRE_EQ(oy.value(), "Nothing here"s);
  }

  TEST_CASE("ExpectTest") {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    REQUIRE_EQ(x.expect("Testing expect"), 2U);

    const result::Result<void, std::string> y = result::Ok();
    REQUIRE_NOTHROW(x.expect("Testing expect"));
  }

  /*
  TEST_CASE("ExpectTerminatedTest") {
    const result::Result<uint32_t, std::string> x = result::Err("emergency failure"s);
    EXPECT_EXIT(x.expect("Testing expect terminated"), ::testing::ExitedWithCode(3), "Testing expect terminated: emergency failure");
  }
  */

  TEST_CASE("ExpectErrTest") {
    const result::Result<uint32_t, std::string> x = result::Err("error 1"s);
    REQUIRE_EQ(x.expect_err("Testing expect_err"), "error 1"s);

    const result::Result<void, std::string> y = result::Err("error 2"s);
    REQUIRE_EQ(y.expect_err("Testing expect_err"), "error 2"s);
  }

  /*
  TEST_CASE("ExpectErrTerminatedTest") {
    const result::Result<uint32_t, std::string> x = result::Ok(42U);
    EXPECT_EXIT(x.expect_err("Testing expect_err terminated"), ::testing::ExitedWithCode(3), "Testing expect_err terminated: 42");

    const result::Result<void, std::string> y = result::Ok();
    EXPECT_EXIT(x.expect_err("Testing expect_err terminated"), ::testing::ExitedWithCode(3), "Testing expect_err terminated");
  }
  */
  TEST_CASE("MapTest") {
    const auto x = generate_container(1U, 12U).map(sum_container);
    REQUIRE(x.is_ok());
    REQUIRE_EQ(x.unwrap(), 78U);

    const auto y = generate_container(10U, 5U).map(sum_container);
    REQUIRE(y.is_err());
    REQUIRE_EQ(y.unwrap_err(), "stop is smaller than start"s);

    const result::Result<void, ErrorCode> z_ok = result::Ok();
    const auto z_ok_r = z_ok.map([]() { return 5U; });
    REQUIRE(z_ok_r.is_ok());
    REQUIRE_EQ(z_ok_r.unwrap(), 5U);

    const result::Result<void, ErrorCode> z_err = result::Err(ErrorCode::ERROR_TWO);
    const auto z_err_r = z_err.map([]() { return 5U; });
    REQUIRE(z_err_r.is_err());
    REQUIRE_EQ(z_err_r.unwrap_err(), ErrorCode::ERROR_TWO);
  }

  TEST_CASE("MapOrTest") {
    const result::Result<std::string, std::string> x = result::Ok("foo"s);
    REQUIRE_EQ(x.map_or(42U, string_length), 3U);

    const result::Result<std::string, std::string> y = result::Err("bar"s);
    REQUIRE_EQ(y.map_or(42U, string_length), 42U);

    const result::Result<void, ErrorCode> z_ok = result::Ok();
    const auto z_ok_r = z_ok.map_or(10U, []() { return 5U; });
    REQUIRE_EQ(z_ok_r, 5U);

    const result::Result<void, ErrorCode> z_err = result::Err(ErrorCode::ERROR_TWO);
    const auto z_err_r = z_err.map_or(10U, []() { return 5U; });
    REQUIRE_EQ(z_err_r, 10U);
  }

  TEST_CASE("MapErrTest") {
    const result::Result<uint32_t, uint32_t> x = result::Ok(2U);
    REQUIRE_EQ(x.map_err(stringify), result::Ok(2U));

    const result::Result<uint32_t, uint32_t> y = result::Err(13U);
    REQUIRE_EQ(y.map_err(stringify), result::Err("error code: 13"s));
  }

  TEST_CASE("MapOrElseTest") {
    const result::Result<std::string, std::string> x = result::Ok("foo"s);
    REQUIRE_EQ(x.map_or_else([](const auto&) { return 0U; }, string_length), 3U);

    const result::Result<std::string, std::string> y = result::Err("bar"s);
    REQUIRE_EQ(y.map_or_else([](const auto&) { return 42U; }, string_length), 42U);

    const result::Result<void, ErrorCode> z_ok = result::Ok();
    const auto z_ok_r = z_ok.map_or_else([](ErrorCode) { return 10U; }, []() { return 5U; });
    REQUIRE_EQ(z_ok_r, 5);

    const result::Result<void, ErrorCode> z_err = result::Err(ErrorCode::ERROR_TWO);
    const auto z_err_r = z_err.map_or_else([](ErrorCode) { return 10U; }, []() { return 5U; });
    REQUIRE_EQ(z_err_r, 10U);
  }

  TEST_CASE("AndTest_stdstring") {
    //error type std::string
    {
      const result::Result<uint32_t, std::string> x = result::Ok(2U);
      const result::Result<std::string, std::string> y = result::Err("late error"s);
      REQUIRE_EQ(x && y, result::Err("late error"));
    }

    {
      const result::Result<uint32_t, std::string> x = result::Err("early error"s);
      const result::Result<std::string, std::string> y = result::Ok("foo"s);
      REQUIRE_EQ(x && y, result::Err("early error"s));
    }

    {
      const result::Result<uint32_t, std::string> x = result::Err("not a 2"s);
      const result::Result<std::string, std::string> y = result::Err("late error"s);
      REQUIRE_EQ(x && y, result::Err("not a 2"));
    }

    {
      const result::Result<uint32_t, std::string> x = result::Ok(2U);
      const result::Result<std::string, std::string> y = result::Ok("different result type"s);
      REQUIRE_EQ(x && y, result::Ok("different result type"s));
    }
  }

  TEST_CASE("AndTest_stringliteral") {
    //error type is const char*
    {
      const auto err = result::Err("late error");
      const result::Result<uint32_t, const char*> x = result::Ok(2U);
      const result::Result<std::string, const char*> y = err;
      REQUIRE_EQ(x && y, err);
      REQUIRE_EQ((x && y).unwrap_err(), y.unwrap_err());
    }

    {
      const auto err = result::Err("early error");
      const result::Result<uint32_t, const char*> x = err;
      const result::Result<std::string, const char*> y = result::Ok("foo"s);
      REQUIRE_EQ(x && y, err);
    }

    {
      const auto err = result::Err("not a 2");
      const result::Result<uint32_t, const char*> x = err;
      const result::Result<std::string, const char*> y = result::Err("late error");
      REQUIRE_EQ(x && y, err);
    }

    {
      const result::Result<uint32_t, const char*> x = result::Ok(2U);
      const result::Result<std::string, const char*> y = result::Ok("different result type"s);
      REQUIRE_EQ(x && y, result::Ok("different result type"s));
    }
  }

  TEST_CASE("AndThenTest") {
    const result::Result<uint32_t, uint32_t> x = result::Ok(2U);
    const result::Result<uint32_t, uint32_t> y = result::Err(3U);
    REQUIRE_EQ(x.and_then(sq).and_then(sq), result::Ok(16U));
    REQUIRE_EQ(x.and_then(sq).and_then(error), result::Err(4U));
    REQUIRE_EQ(x.and_then(error).and_then(sq), result::Err(2U));
    REQUIRE_EQ(y.and_then(sq).and_then(sq), result::Err(3U));
  }

  TEST_CASE("OrTest") {
    {
      const result::Result<uint32_t, std::string> x = result::Ok(2U);
      const result::Result<uint32_t, std::string> y = result::Err("late error"s);
      REQUIRE_EQ(x || y, result::Ok(2U));
    }

    {
      const result::Result<uint32_t, std::string> x = result::Err("early error"s);
      const result::Result<uint32_t, std::string> y = result::Ok(2U);
      REQUIRE_EQ(x || y, result::Ok(2U));
    }

    {
      const result::Result<uint32_t, std::string> x = result::Err("not a 2"s);
      const result::Result<uint32_t, std::string> y = result::Err("late error"s);
      REQUIRE_EQ(x || y, result::Err("late error"));
    }

    {
      const result::Result<uint32_t, std::string> x = result::Ok(2U);
      const result::Result<uint32_t, std::string> y = result::Ok(100U);
      REQUIRE_EQ(x || y, result::Ok(2U));
    }
  }

  TEST_CASE("OrElseTest") {
    const result::Result<uint32_t, uint32_t> x = result::Ok(2U);
    const result::Result<uint32_t, uint32_t> y = result::Err(3U);
    REQUIRE_EQ(x.or_else(sq).or_else(sq), result::Ok(2U));
    REQUIRE_EQ(x.or_else(error).or_else(sq), result::Ok(2U));
    REQUIRE_EQ(y.or_else(sq).or_else(error), result::Ok(9U));
    REQUIRE_EQ(y.or_else(error).or_else(error), result::Err(3U));
  }

  TEST_CASE("UnwrapTest") {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    REQUIRE_EQ(x.unwrap(), 2U);

    //const result::Result<uint32_t, std::string> y = result::Err("emergency failure"s);
    //EXPECT_EXIT(static_cast<void>(y.unwrap()), ::testing::ExitedWithCode(3), "Attempting to unwrap an Err Result: emergency failure"); // panics with `emergency failure`
  }

  TEST_CASE("UnwrapErrTest") {
    //const result::Result<uint32_t, std::string> x = result::Ok(2U);
    //EXPECT_EXIT(static_cast<void>(x.unwrap_err()), ::testing::ExitedWithCode(3), "Attempting to unwrap_err an Ok Result: 2"); // panics

    const result::Result<uint32_t, std::string> y = result::Err("emergency failure"s);
    REQUIRE_EQ(y.unwrap_err(), "emergency failure");
  }

  TEST_CASE("UnwrapOrTest") {
    const auto value = 2;
    const result::Result<uint32_t, std::string> x = result::Ok(9U);
    REQUIRE_EQ(x.unwrap_or(value), 9U);

    const result::Result<uint32_t, std::string> y = result::Err("error"s);
    REQUIRE_EQ(y.unwrap_or(value), value);
  }

  TEST_CASE("UnwrapOrDefaultTest") {
    const result::Result<uint32_t, std::string> x = result::Ok(9U);
    REQUIRE_EQ(x.unwrap_or_default(), 9U);

    const result::Result<std::string, std::string> y = result::Err("error"s);
    REQUIRE_EQ(y.unwrap_or_default(), std::string());
  }

  TEST_CASE("UnwrapOrElseTest") {
    const result::Result<size_t, std::string> x = result::Ok(2ULL);
    REQUIRE_EQ(x.unwrap_or_else(string_length), 2ULL);

    const result::Result<size_t, std::string> y = result::Err("foo"s);
    REQUIRE_EQ(y.unwrap_or_else(string_length), 3ULL);
  }

  TEST_CASE("ContainsTest") {
    const result::Result<int, ErrorCode> x = result::Ok(5);
    REQUIRE(x.contains(5));
    REQUIRE_FALSE(x.contains(4));

    const result::Result<int, ErrorCode> y = result::Err(ErrorCode::ERROR_ONE);
    REQUIRE_FALSE(x.contains(4));
  }

  TEST_CASE("ContainsErrTest") {
    const result::Result<int, ErrorCode> x = result::Ok(5);
    REQUIRE_FALSE(x.contains_err(ErrorCode::ERROR_THREE));

    const result::Result<int, std::string> y = result::Err("some error"s);
    REQUIRE(y.contains_err("some error"));
    REQUIRE_FALSE(y.contains_err("other error"));
  }

  TEST_CASE("TransposeTest") {
    const result::Result<std::optional<int32_t>, std::string> x = result::Ok(std::make_optional(5));
    const std::optional<result::Result<int32_t, std::string>> y = std::make_optional<result::Result<int32_t, std::string>>(result::Ok(5));
    const auto t = x.transpose();
    REQUIRE(t);
    REQUIRE_EQ(t, y);
  }

  TEST_CASE("NonDefaultCtorTypeTest") {
    struct type_with_non_default_ctor {
      explicit type_with_non_default_ctor(int v) : i(v) {
      }
      type_with_non_default_ctor() = delete;
      type_with_non_default_ctor(const type_with_non_default_ctor&) = default;
      type_with_non_default_ctor(type_with_non_default_ctor&&) = default;
      type_with_non_default_ctor& operator=(const type_with_non_default_ctor&) = default;
      type_with_non_default_ctor& operator=(type_with_non_default_ctor&&) = default;
      ~type_with_non_default_ctor() = default;

      int i;
    };

    //result::Result<type_with_non_default_ctor, int> x1; //it should not compile
    const result::Result<type_with_non_default_ctor, int> x2 = result::Ok(type_with_non_default_ctor(5));
    const auto lval = type_with_non_default_ctor(3);
    const result::Result<type_with_non_default_ctor, int> x3 = result::Ok(lval);
    REQUIRE(x3.is_ok());
    const auto x3_val = x3.unwrap();
    REQUIRE_EQ(x3_val.i, 3);
    //const auto x3_val_def = x3.unwrap_or_default(); //it should not compile

    const result::Result<type_with_non_default_ctor, int> x4 = result::Err(1);
    REQUIRE(x4.is_err());
  }
}
