#include "gtest/gtest.h"
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
  return std::accumulate(std::begin(container), std::end(container), value_type{});
};

auto stringify = [](auto x) {
  std::stringstream ss;
  ss << "error code: " << x;
  return ss.str();
};

auto string_length = [](const auto& str) { return str.length(); };

auto sq = [](uint32_t x) -> result::Result<uint32_t, uint32_t> { return result::Ok(x * x); };
auto error = [](uint32_t x) -> result::Result<uint32_t, uint32_t> { return result::Err(x); };

struct type_with_no_copy_ctor_and_no_copy_assignment {
  type_with_no_copy_ctor_and_no_copy_assignment() = default;
  type_with_no_copy_ctor_and_no_copy_assignment(const type_with_no_copy_ctor_and_no_copy_assignment&) = delete;
  type_with_no_copy_ctor_and_no_copy_assignment(type_with_no_copy_ctor_and_no_copy_assignment&&) = default;
  type_with_no_copy_ctor_and_no_copy_assignment& operator=(const type_with_no_copy_ctor_and_no_copy_assignment&) = delete;
  type_with_no_copy_ctor_and_no_copy_assignment& operator=(type_with_no_copy_ctor_and_no_copy_assignment&&) = default;
  ~type_with_no_copy_ctor_and_no_copy_assignment() = default;
  uint32_t ui;
};

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const type_with_no_copy_ctor_and_no_copy_assignment& val) {
  os << val.ui;
  return os;
}

struct type_with_no_move_ctor_and_no_move_assignment {
  type_with_no_move_ctor_and_no_move_assignment() = default;
  type_with_no_move_ctor_and_no_move_assignment(const type_with_no_move_ctor_and_no_move_assignment&) = default;
  type_with_no_move_ctor_and_no_move_assignment(type_with_no_move_ctor_and_no_move_assignment&&) = delete;
  type_with_no_move_ctor_and_no_move_assignment& operator=(const type_with_no_move_ctor_and_no_move_assignment&) = default;
  type_with_no_move_ctor_and_no_move_assignment& operator=(type_with_no_move_ctor_and_no_move_assignment&&) = delete;
  ~type_with_no_move_ctor_and_no_move_assignment() = default;
  uint32_t ui;
};

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const type_with_no_move_ctor_and_no_move_assignment& val) {
  os << val.ui;
  return os;
}

TEST(ResultTests, TypeWithNoCopyCtorAndNoCopyAssignment) {
  /*
  type_with_no_copy_ctor_and_no_copy_assignment var1;
  //type_with_no_copy_ctor_and_no_copy_assignment var2 = var1; //it should not compile
  type_with_no_copy_ctor_and_no_copy_assignment var3;
  var3 = var1; //it should not compile
  */

  //auto val = type_with_no_copy_ctor_and_no_copy_assignment{ 10U };
  //const result::Result<type_with_no_copy_ctor_and_no_copy_assignment, uint32_t> x = result::Ok(val); //it should not compile
  const result::Result<type_with_no_copy_ctor_and_no_copy_assignment, uint32_t> x = result::Ok(type_with_no_copy_ctor_and_no_copy_assignment{ 10U });
  EXPECT_TRUE(x.is_ok());
  //auto var = x.unwrap(); //it should not compile
  EXPECT_EQ(x.unwrap().ui, 10U);
}

TEST(ResultTests, TypeWithNoMoveCtorAndNoMoveAssignment) {
  /*
  type_with_no_move_ctor_and_no_move_assignment var1;
  //type_with_no_move_ctor_and_no_move_assignment var2 = std::move(var1); //it should not compile
  type_with_no_move_ctor_and_no_move_assignment var3;
  var3 = std::move(var1); //it should not compile
  */

  auto val = type_with_no_move_ctor_and_no_move_assignment{ 10U };
  const result::Result<type_with_no_move_ctor_and_no_move_assignment, uint32_t> x = result::Ok(val);
  //const result::Result<type_with_no_move_ctor_and_no_move_assignment, uint32_t> x = result::Ok(type_with_no_move_ctor_and_no_move_assignment{ 10U }); //it should not compile
  EXPECT_TRUE(x.is_ok());
  const type_with_no_move_ctor_and_no_move_assignment var = x.unwrap();
  EXPECT_EQ(var.ui, 10U);
}

TEST(ResultTests, IsOk) {
  const result::Result<double, ErrorCode> x = result::Ok(3.14);
  EXPECT_TRUE(x.is_ok());
  EXPECT_FALSE(x.is_err());

  const result::Result<double, ErrorCode> y{};
  EXPECT_TRUE(y.is_ok());
  EXPECT_FALSE(y.is_err());

  const result::Result<void, ErrorCode> z = result::Ok();
  EXPECT_TRUE(z.is_ok());
  EXPECT_FALSE(z.is_err());

  const result::Result<void, ErrorCode> i{};
  EXPECT_TRUE(i.is_ok());
  EXPECT_FALSE(i.is_err());

  result::Result<double, ErrorCode> j = result::Ok<double>(3);
  EXPECT_TRUE(j.is_ok());
  EXPECT_FALSE(j.is_err());
}

TEST(ResultTests, IsErr) {
  const result::Result<double, ErrorCode> x = result::Err(ErrorCode::ERROR_ONE);
  EXPECT_TRUE(x.is_err());
  EXPECT_FALSE(x.is_ok());
}

TEST(ResultTests, Ok) {
  const result::Result<uint32_t, std::string> x = result::Ok(2U);
  const auto ox = x.ok();
  EXPECT_TRUE(ox.has_value());
  EXPECT_EQ(ox.value(), 2U);

  const result::Result<uint32_t, std::string> y = result::Err("Nothing here"s);
  const auto oy = y.ok();
  EXPECT_FALSE(oy.has_value());
}

TEST(ResultTests, Err) {
  const result::Result<uint32_t, std::string> x = result::Ok(2U);
  const auto ox = x.err();
  EXPECT_FALSE(ox.has_value());

  const result::Result<uint32_t, std::string> y = result::Err("Nothing here"s);
  const auto oy = y.err();
  EXPECT_TRUE(oy.has_value());
  EXPECT_EQ(oy.value(), "Nothing here"s);
}

TEST(ResultTests, Expect) {
  const result::Result<uint32_t, std::string> x = result::Ok(2U);
  EXPECT_EQ(x.expect("Testing expect"), 2U);

  const result::Result<void, std::string> y = result::Ok();
  //there is no expect function of top of Result<T, E> where T is void
}

TEST(ResultTests, ExpectTerminated) {
  const result::Result<uint32_t, std::string> x = result::Err("emergency failure"s);
  EXPECT_EXIT(x.expect("Testing expect terminated"), ::testing::ExitedWithCode(EXIT_FAILURE), "Testing expect terminated: emergency failure");
}

TEST(ResultTests, ExpectErr) {
  const result::Result<uint32_t, std::string> x = result::Err("error 1"s);
  EXPECT_EQ(x.expect_err("Testing expect_err"), "error 1"s);

  const result::Result<void, std::string> y = result::Err("error 2"s);
  EXPECT_EQ(y.expect_err("Testing expect_err"), "error 2"s);
}

TEST(ResultTests, ExpectErrTerminated) {
  const result::Result<uint32_t, std::string> x = result::Ok(42U);
  EXPECT_EXIT(x.expect_err("Testing expect_err terminated"), ::testing::ExitedWithCode(EXIT_FAILURE), "Testing expect_err terminated: 42");
}

TEST(ResultTests, ExpectErrTerminatedResultVoid) {
  const result::Result<void, std::string> x = result::Ok();
  EXPECT_EXIT(x.expect_err("Testing expect_err terminated"), ::testing::ExitedWithCode(EXIT_FAILURE), "Testing expect_err terminated");
}

TEST(ResultTests, Map) {
  const auto x = generate_container(1U, 12U).map(sum_container);
  EXPECT_TRUE(x.is_ok());
  EXPECT_EQ(x.unwrap(), 78U);

  const auto y = generate_container(10U, 5U).map(sum_container);
  EXPECT_TRUE(y.is_err());
  EXPECT_EQ(y.unwrap_err(), "stop is smaller than start"s);

  const result::Result<void, ErrorCode> z_ok = result::Ok();
  const auto z_ok_r = z_ok.map([]() { return 5U; });
  EXPECT_TRUE(z_ok_r.is_ok());
  EXPECT_EQ(z_ok_r.unwrap(), 5U);

  const result::Result<void, ErrorCode> z_err = result::Err(ErrorCode::ERROR_TWO);
  const auto z_err_r = z_err.map([]() { return 5U; });
  EXPECT_TRUE(z_err_r.is_err());
  EXPECT_EQ(z_err_r.unwrap_err(), ErrorCode::ERROR_TWO);
}

TEST(ResultTests, MapOr) {
  const result::Result<std::string, std::string> x = result::Ok("foo"s);
  EXPECT_EQ(x.map_or(42U, string_length), 3U);

  const result::Result<std::string, std::string> y = result::Err("bar"s);
  EXPECT_EQ(y.map_or(42U, string_length), 42U);

  const result::Result<void, ErrorCode> z_ok = result::Ok();
  const auto z_ok_r = z_ok.map_or(10U, []() { return 5U; });
  EXPECT_EQ(z_ok_r, 5U);

  const result::Result<void, ErrorCode> z_err = result::Err(ErrorCode::ERROR_TWO);
  const auto z_err_r = z_err.map_or(10U, []() { return 5U; });
  EXPECT_EQ(z_err_r, 10U);
}

TEST(ResultTests, MapErr) {
  const result::Result<uint32_t, uint32_t> x = result::Ok(2U);
  EXPECT_EQ(x.map_err(stringify), result::Ok(2U));

  const result::Result<uint32_t, uint32_t> y = result::Err(13U);
  EXPECT_EQ(y.map_err(stringify), result::Err("error code: 13"s));
}

TEST(ResultTests, MapOrElse) {
  const result::Result<std::string, std::string> x = result::Ok("foo"s);
  EXPECT_EQ(x.map_or_else([](const auto&) { return 0U; }, string_length),  3U);

  const result::Result<std::string, std::string> y = result::Err("bar"s);
  EXPECT_EQ(y.map_or_else([](const auto&) { return 42U; }, string_length), 42U);

  const result::Result<void, ErrorCode> z_ok = result::Ok();
  const auto z_ok_r = z_ok.map_or_else([](ErrorCode) { return 10U; }, []() { return 5U; });
  EXPECT_EQ(z_ok_r, 5);

  const result::Result<void, ErrorCode> z_err = result::Err(ErrorCode::ERROR_TWO);
  const auto z_err_r = z_err.map_or_else([](ErrorCode) { return 10U; }, []() { return 5U; });
  EXPECT_EQ(z_err_r, 10U);
}

TEST(ResultTests, And_stdstring) {
//error type std::string
  {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    const result::Result<std::string, std::string> y = result::Err("late error"s);
    EXPECT_EQ(x && y, result::Err("late error"));
  }

  {
    const result::Result<uint32_t, std::string> x = result::Err("early error"s);
    const result::Result<std::string, std::string> y = result::Ok("foo"s);
    EXPECT_EQ(x && y, result::Err("early error"s));
  }

  {
    const result::Result<uint32_t, std::string> x = result::Err("not a 2"s);
    const result::Result<std::string, std::string> y = result::Err("late error"s);
    EXPECT_EQ(x && y, result::Err("not a 2"));
  }

  {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    const result::Result<std::string, std::string> y = result::Ok("different result type"s);
    EXPECT_EQ(x && y, result::Ok("different result type"s));
  }
}

TEST(ResultTests, And_stringliteral) {
  //error type is const char*
  {
    const auto err = result::Err("late error");
    const result::Result<uint32_t, const char*> x = result::Ok(2U);
    const result::Result<std::string, const char*> y = err;
    EXPECT_EQ(x && y, err);
    EXPECT_EQ((x && y).unwrap_err(), y.unwrap_err());
  }

  {
    const auto err = result::Err("early error");
    const result::Result<uint32_t, const char*> x = err;
    const result::Result<std::string, const char*> y = result::Ok("foo"s);
    EXPECT_EQ(x && y, err);
  }

  {
    const auto err = result::Err("not a 2");
    const result::Result<uint32_t, const char*> x = err;
    const result::Result<std::string, const char*> y = result::Err("late error");
    EXPECT_EQ(x && y, err);
  }

  {
    const result::Result<uint32_t, const char*> x = result::Ok(2U);
    const result::Result<std::string, const char*> y = result::Ok("different result type"s);
    EXPECT_EQ(x && y, result::Ok("different result type"s));
  }
}

TEST(ResultTests, AndThen) {
  const result::Result<uint32_t, uint32_t> x = result::Ok(2U);
  const result::Result<uint32_t, uint32_t> y = result::Err(3U);
  EXPECT_EQ(x.and_then(sq).and_then(sq), result::Ok(16U));
  EXPECT_EQ(x.and_then(sq).and_then(error), result::Err(4U));
  EXPECT_EQ(x.and_then(error).and_then(sq), result::Err(2U));
  EXPECT_EQ(y.and_then(sq).and_then(sq), result::Err(3U));
}

TEST(ResultTests, Or) {
  {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    const result::Result<uint32_t, std::string> y = result::Err("late error"s);
    EXPECT_EQ(x || y, result::Ok(2U));
  }

  {
    const result::Result<uint32_t, std::string> x = result::Err("early error"s);
    const result::Result<uint32_t, std::string> y = result::Ok(2U);
    EXPECT_EQ(x || y, result::Ok(2U));
  }

  {
    const result::Result<uint32_t, std::string> x = result::Err("not a 2"s);
    const result::Result<uint32_t, std::string> y = result::Err("late error"s);
    EXPECT_EQ(x || y, result::Err("late error"));
  }

  {
    const result::Result<uint32_t, std::string> x = result::Ok(2U);
    const result::Result<uint32_t, std::string> y = result::Ok(100U);
    EXPECT_EQ(x || y, result::Ok(2U));
  }
}

TEST(ResultTests, OrElse) {
  const result::Result<uint32_t, uint32_t> x = result::Ok(2U);
  const result::Result<uint32_t, uint32_t> y = result::Err(3U);
  EXPECT_EQ(x.or_else(sq).or_else(sq), result::Ok(2U));
  EXPECT_EQ(x.or_else(error).or_else(sq), result::Ok(2U));
  EXPECT_EQ(y.or_else(sq).or_else(error), result::Ok(9U));
  EXPECT_EQ(y.or_else(error).or_else(error), result::Err(3U));
}

TEST(ResultTests, Unwrap) {
  const result::Result<uint32_t, std::string> x = result::Ok(2U);
  EXPECT_EQ(x.unwrap(), 2U);
}

TEST(ResultTests, UnwrapTerminated) {
  const result::Result<uint32_t, std::string> x = result::Err("emergency failure"s);
  EXPECT_EXIT(static_cast<void>(x.unwrap()), ::testing::ExitedWithCode(EXIT_FAILURE), "Attempting to unwrap an Err Result: emergency failure"); // panics with `emergency failure`
}

TEST(ResultTests, UnwrapErrTerminated) {
  const result::Result<uint32_t, std::string> x = result::Ok(2U);
  EXPECT_EXIT(static_cast<void>(x.unwrap_err()), ::testing::ExitedWithCode(EXIT_FAILURE), "Attempting to unwrap_err an Ok Result: 2"); // panics
}

TEST(ResultTests, UnwrapErr) {
  const result::Result<uint32_t, std::string> x = result::Err("emergency failure"s);
  EXPECT_EQ(x.unwrap_err(), "emergency failure");
}

TEST(ResultTests, UnwrapOr) {
   const auto value = 2;
    const result::Result<uint32_t, std::string> x = result::Ok(9U);
    EXPECT_EQ(x.unwrap_or(value), 9U);

    const result::Result<uint32_t, std::string> y = result::Err("error"s);
    EXPECT_EQ(y.unwrap_or(value), value);
  }

TEST(ResultTests, UnwrapOrDefault) {
  const result::Result<uint32_t, std::string> x = result::Ok(9U);
  EXPECT_EQ(x.unwrap_or_default(), 9U);

  const result::Result<std::string, std::string> y = result::Err("error"s);
  EXPECT_EQ(y.unwrap_or_default(), std::string());
}

TEST(ResultTests, UnwrapOrElse) {
  const result::Result<size_t, std::string> x = result::Ok(static_cast<size_t>(2U));
  EXPECT_EQ(x.unwrap_or_else(string_length), 2ULL);

  const result::Result<size_t, std::string> y = result::Err("foo"s);
  EXPECT_EQ(y.unwrap_or_else(string_length), 3ULL);
}

TEST(ResultTests, Contains) {
  const result::Result<int, ErrorCode> x = result::Ok(5);
  EXPECT_TRUE(x.contains(5));
  EXPECT_FALSE(x.contains(4));

  const result::Result<int, ErrorCode> y = result::Err(ErrorCode::ERROR_ONE);
  EXPECT_FALSE(x.contains(4));
}

TEST(ResultTests, ContainsErr) {
  const result::Result<int, ErrorCode> x = result::Ok(5);
  EXPECT_FALSE(x.contains_err(ErrorCode::ERROR_THREE));

  const result::Result<int, std::string> y = result::Err("some error"s);
  EXPECT_TRUE(y.contains_err("some error"));
  EXPECT_FALSE(y.contains_err("other error"));
}

TEST(ResultTests, Transpose) {
  const result::Result<std::optional<int32_t>, std::string> x = result::Ok(std::make_optional(5));
  const std::optional<result::Result<int32_t, std::string>> y = std::make_optional<result::Result<int32_t, std::string>>(result::Ok(5));
  const auto t = x.transpose();
  EXPECT_TRUE(t);
  EXPECT_EQ(t, y);
}

TEST(ResultTests, NonDefaultCtorType) {
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
  const auto lvalue = type_with_non_default_ctor(3);
  const result::Result<type_with_non_default_ctor, int> x3 = result::Ok(lvalue);
  EXPECT_TRUE(x3.is_ok());
  const auto x3_val = x3.unwrap();
  EXPECT_EQ(x3_val.i, 3);
  //const auto x3_val_def = x3.unwrap_or_default(); //it should not compile

  const result::Result<type_with_non_default_ctor, int> x4 = result::Err(1);
  EXPECT_TRUE(x4.is_err());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
