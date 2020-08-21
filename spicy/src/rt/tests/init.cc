// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#include <doctest/doctest.h>

#include <map>
#include <optional>
#include <vector>

#include <hilti/rt/init.h>
#include <hilti/rt/types/port.h>

#include <spicy/rt/global-state.h>
#include <spicy/rt/init.h>
#include <spicy/rt/mime.h>
#include <spicy/rt/parser.h>
#include <spicy/rt/typedefs.h>

using namespace spicy::rt;

TEST_SUITE_BEGIN("Init");

TEST_CASE("init") {
    done(); // Noop if not initialized.
    CHECK_EQ(detail::__global_state, nullptr);

    hilti::rt::init(); // Noop if already initialized.


    SUBCASE("w/o parser setup") {
        init();

        const auto* gs = detail::__global_state;
        REQUIRE_NE(gs, nullptr);
        CHECK(gs->runtime_is_initialized);
        CHECK_EQ(gs->default_parser, std::nullopt);
        CHECK(gs->parsers_by_name.empty());
        CHECK(gs->parsers_by_mime_type.empty());

        init();

        CHECK_EQ(detail::__global_state, gs);
    }

    SUBCASE("single parser") {
        const Parser parser("Parser", Parse1Function(), Parse2Function<int>(), Parse3Function(), nullptr,
                            "Parser: description", {MIMEType("foo/bar")},
                            {ParserPort{{hilti::rt::Port(4040, hilti::rt::Protocol::TCP), Direction::Both}}});
        detail::globalState()->parsers.push_back(&parser);

        init();

        const auto* gs = detail::__global_state;
        REQUIRE_NE(gs, nullptr);
        CHECK_EQ(gs->default_parser, &parser);

        CHECK_EQ(gs->parsers_by_name,
                 std::map<std::string, std::vector<const Parser*>>({{parser.name, {&parser}},
                                                                    {"4040/tcp", {&parser}},
                                                                    {"4040/tcp%orig", {&parser}},
                                                                    {"4040/tcp%resp", {&parser}},
                                                                    {parser.mime_types.at(0), {&parser}}}));
        CHECK(gs->parsers_by_mime_type.empty()); // Never updated.
    }

    SUBCASE("multiple parsers") {
        const Parser parser1("Parser1", Parse1Function(), Parse2Function<int>(), Parse3Function(), nullptr,
                             "Parser1: description", {MIMEType("foo/bar")},
                             {ParserPort{{hilti::rt::Port(4040, hilti::rt::Protocol::TCP), Direction::Originator}}});
        const Parser parser2("Parser2", Parse1Function(), Parse2Function<int>(), Parse3Function(), nullptr,
                             "Parser2: description", {MIMEType("foo/*")},
                             {ParserPort{{hilti::rt::Port(4040, hilti::rt::Protocol::TCP), Direction::Responder}}});
        detail::globalState()->parsers.push_back(&parser1);
        detail::globalState()->parsers.push_back(&parser2);

        init();

        const auto* gs = detail::__global_state;
        REQUIRE_NE(gs, nullptr);
        CHECK_EQ(gs->default_parser, std::nullopt);

        CAPTURE(gs->parsers.size());
        CAPTURE(gs->parsers_by_name.size());
        CHECK_EQ(gs->parsers_by_name,
                 std::map<std::string, std::vector<const Parser*>>({{parser1.name, {&parser1}},
                                                                    {"4040/tcp%orig", {&parser1}},
                                                                    {parser1.mime_types.at(0), {&parser1}},
                                                                    {parser2.name, {&parser2}},
                                                                    {"4040/tcp%resp", {&parser2}}}));
        CHECK(gs->parsers_by_mime_type.empty()); // Never updated.
    }
}

TEST_CASE("isInitialized") {
    done(); // Noop if not initialized.
    REQUIRE_FALSE(isInitialized());

    init();

    CHECK(isInitialized());
}

TEST_CASE("done") {
    hilti::rt::init();
    init();
    REQUIRE_NE(detail::__global_state, nullptr);

    done();
    CHECK_EQ(detail::__global_state, nullptr);

    done();
    CHECK_EQ(detail::__global_state, nullptr);
}

TEST_SUITE_END();
