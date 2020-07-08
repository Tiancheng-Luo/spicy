#include <doctest/doctest.h>

#include <string>
#include <string_view>

#include <hilti/rt/extension-points.h>
#include <hilti/rt/types/result.h>
#include <hilti/rt/types/string.h>

using namespace hilti::rt;

TEST_SUITE_BEGIN("Result");

TEST_CASE_TEMPLATE("default constructed is error", T, Nothing, bool, std::string) {
    Result<T> r;
    CHECK(! r);
    CHECK(r.errorOrThrow() == result::Error("<result not initialized>"));
}

TEST_CASE_TEMPLATE("conversion to bool", T, Nothing, bool, std::string) {
    Result<T> r;
    CHECK(! r);

    if constexpr ( std::is_same_v<T, void> )
        r = Nothing();
    else
        r = T{};

    CHECK(r);
}

TEST_CASE("equal") {
    CHECK_EQ(Result(42), Result(42));
    CHECK_EQ(Result(0), Result(0));
    CHECK_EQ(Result<int>(result::Error("foo")), Result<int>(result::Error("foo")));
}

TEST_CASE("not equal") {
    CHECK_NE(Result(42), Result(0));
    CHECK_NE(Result(42), Result<int>(result::Error("foo")));
}

TEST_CASE("valueOrThrow") {
    SUBCASE("const") {
        const auto r1 = Result<int>(0);
        const auto r2 = Result<int>();
        const auto r3 = Result<int>(result::Error("foo"));

        CHECK_EQ(r1.valueOrThrow(), 0);
        CHECK_THROWS_WITH_AS(r2.valueOrThrow(), "<result not initialized>", const result::NoResult&);
        CHECK_THROWS_WITH_AS(r3.valueOrThrow(), "foo", const result::NoResult&);
    }

    SUBCASE("non const") {
        auto r1 = Result<int>(0);
        auto r2 = Result<int>();
        auto r3 = Result<int>(result::Error("foo"));

        CHECK_EQ(r1.valueOrThrow(), 0);
        CHECK_THROWS_WITH_AS(r2.valueOrThrow(), "<result not initialized>", const result::NoResult&);
        CHECK_THROWS_WITH_AS(r3.valueOrThrow(), "foo", const result::NoResult&);

        r1.valueOrThrow() += 42;
        CHECK_EQ(r1, Result(42));
    }
}

TEST_CASE("to_string_for_print") {
    CHECK_EQ(to_string_for_print(Result<std::string>("abc")), "abc");
    CHECK_EQ(to_string_for_print(Result<std::string>()), "<error: <result not initialized>>");

    CHECK_EQ(to_string_for_print(Result<std::string_view>("abc")), "abc");
    CHECK_EQ(to_string_for_print(Result<std::string>()), "<error: <result not initialized>>");
}

TEST_SUITE_END();
