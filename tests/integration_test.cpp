#include <ApprovalTests.hpp>
#include <catch2/catch.hpp>

#include "eml.hpp"

void verify_eml(const std::string& source)
{
  eml::GarbageCollector gc{};

  eml::CompilerConfig config = {eml::SameScopeShadowing::allow};
  eml::Compiler compiler{gc, config};
  eml::VM vm{gc};

  std::stringstream ss;
  ss << "Input:\n" << source << "\n\n";

  if (!source.empty()) {
    compiler.compile(source)
        .map([&vm, &ss](auto tuple) {
          const auto [bytecode, type] = tuple;

          ss << "Compile to:\n" << bytecode << '\n';

          const auto result = vm.interpret(bytecode);
          REQUIRE(result);
          ss << "Executes to:\n" << eml::to_string(type, *result) << '\n';
        })
        .map_error([&ss](const auto& errors) {
          ss << "Error message:\n";
          std::for_each(std::begin(errors), std::end(errors),
                        [&ss](auto e) { ss << eml::to_string(e); });
        });
    ApprovalTests::Approvals::verify(ss.str());
  }
}

void verify_for_documentation(const std::string& source)
{
  eml::GarbageCollector gc{};

  eml::CompilerConfig config = {eml::SameScopeShadowing::allow};
  eml::Compiler compiler{gc, config};
  eml::VM vm{gc};

  std::stringstream ss;
  ss << "Input:\n" << source << "\n\n";

  if (!source.empty()) {
    compiler.compile(source)
        .map([&vm, &ss](auto tuple) {
          const auto [bytecode, type] = tuple;

          const auto result = vm.interpret(bytecode);
          REQUIRE(result);
          ss << "Executes to:\n" << eml::to_string(type, *result) << '\n';
        })
        .map_error([&ss](const auto& errors) {
          ss << "Error message:\n";
          std::for_each(std::begin(errors), std::end(errors),
                        [&ss](auto e) { ss << eml::to_string(e); });
        });
    ApprovalTests::Approvals::verify(
        ss.str(),
        ApprovalTests::Options{}.fileOptions().withFileExtension(".docs.txt"));
  }
}

TEST_CASE("Integration Test")
{
  // begin-snippet: example_from_code
  const std::string s = "if (5 > 1) {2 + 3} else {4 - 6}";
  // end-snippet
  verify_eml(s);
  verify_for_documentation(s);
}

TEST_CASE("Test Error")
{
  verify_eml("if (5 > 1 {2 + 3} else {4 - 6}");
}

TEST_CASE("Test with String")
{
  const std::string s = R"("Hello, " ++ "world")";
  verify_eml(s);
}

TEST_CASE("Test with Arithmetics")
{
  verify_eml("1 + 3 / 4 * 5 - 10");
}