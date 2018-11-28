#include <catch2/catch.hpp>

#include <array>

#include "scanner.hpp"

using eml::Scanner;
using eml::Token;
using eml::token_type;

TEST_CASE("scanner")
{

  GIVEN("A string of keywords")
  {
    const auto keywords =
        R"(and async await case class def else extern false for if let not
        or print return this true () unsafe variant)";

    Scanner s{keywords};

    WHEN("Scan the string")
    {
      THEN("Should get corerect keywords tokens")
      {
        std::array expected{
            Token{token_type::keyword_and, "and"},
            Token{token_type::keyword_async, "async"},
            Token{token_type::keyword_await, "await"},
            Token{token_type::keyword_case, "case"},
            Token{token_type::keyword_class, "class"},
            Token{token_type::keyword_def, "def"},
            Token{token_type::keyword_else, "else"},
            Token{token_type::keyword_extern, "extern"},
            Token{token_type::keyword_false, "false"},
            Token{token_type::keyword_for, "for"},
            Token{token_type::keyword_if, "if"},
            Token{token_type::keyword_let, "let"},
            Token{token_type::keyword_not, "not"},
            Token{token_type::keyword_or, "or"},
            Token{token_type::keyword_print, "print"},
            Token{token_type::keyword_return, "return"},
            Token{token_type::keyword_this, "this"},
            Token{token_type::keyword_true, "true"},
            Token{token_type::keyword_unit, "()"},
            Token{token_type::keyword_unsafe, "unsafe"},
            Token{token_type::keyword_variant, "variant"},
        };

        auto scanner_itr = std::begin(s);

        for (auto expected_token : expected) {
          REQUIRE(*scanner_itr == expected_token);
          ++scanner_itr;
        }
        REQUIRE(scanner_itr == std::end(s));
      }
    }
  }
}

TEST_CASE("Error handling of the scanner")
{
  GIVEN("A string contains unknown characters")
  {
    Scanner s{R"(let # = 3)"};
    WHEN("Scan the string")
    {
      THEN("Correct identify the error, but scan the remaining part correctly")
      {
        std::array expected{
            Token{token_type::keyword_let, "let"},
            Token{token_type::error, "Unexpected character."},
            Token{token_type::equal, "="},
            Token{token_type::number_literal, "3"},
        };

        auto scanner_itr = std::begin(s);
        for (auto expected_token : expected) {
          REQUIRE(*scanner_itr == expected_token);
          ++scanner_itr;
        }
        REQUIRE(scanner_itr == std::end(s));
      }
    }
  }

  GIVEN("None-terminate string")
  {
    Scanner s{R"("Hello world)"};
    THEN("Should detect the problem")
    {
      Token expected{token_type::error, "Unterminated string."};
      REQUIRE(*s.begin() == expected);
    }
  }
}
