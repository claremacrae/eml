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
          ApprovalTests::Approvals::verify(ss.str());
        })
        .map_error([](const auto& errors) {
          std::for_each(std::begin(errors), std::end(errors),
                        [](auto e) { std::clog << eml::to_string(e); });
        });
  }
}

TEST_CASE("Integration Test")
{
  verify_eml("if (5 > 1) {2 + 3} else {4 - 6}");
}