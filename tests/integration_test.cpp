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

// This demos how to approval multiple inputs in a single test.
// In practice, you would probably put most of it in a helper function,
// maybe verify_emls() - or something more meaningful.
TEST_CASE("Test Errors")
{
  // A tip for your main.cpp
  // -----------------------
  // Put the approved and received files in a sub-directory called approval_tests/:
  // You could also put this in your main.cpp - at global scope, so it applies to every test.
  // Even better, you could pass in the name of the current source file (without file-extension)
  // and then there would be a separate sub-directory for the approved files from each source file.
  // See https://github.com/approvals/ApprovalTests.cpp/blob/master/doc/Configuration.md/#using-sub-directories-for-approved-files
  auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("approval_tests");

  // Create a container of test inputs - to make it easier still to
  // add new test cases and improve test coverage:
  std::vector<std::string> inputs = {
      "if (5 > 1 {2 + 3} else {4 - 6}", // missing )
      "if (5 > 1) {2 + 3 else {4 - 6}" // missing }
  };

  for (const auto& input: inputs)
  {
    // Using SECTION allows us to have multiple verify() calls in a single test case.
    // See https://github.com/approvals/ApprovalTests.cpp/blob/master/doc/MultipleOutputFilesPerTest.md#catch2
    SECTION(input)
    {
      verify_eml(input);
    }
  }
}

//TEST_CASE("Test with String")
//{
//  const std::string s = R"("Hello, " ++ "world")";
//  verify_eml(s);
//}

TEST_CASE("Test with Arithmetics")
{
  verify_eml("1 + 3 / 4 * 5 - 10");
}