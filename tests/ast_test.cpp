#include "ast.hpp"
#include "compiler.hpp"
#include "debug.hpp"

#include "vm_test_util.hpp"

#include <catch2/catch.hpp>

TEST_CASE("AST visiting and printing")
{
  namespace ast = eml::ast;

  GIVEN("A Unary negate expression -10")
  {
    WHEN("Invoke the Ast Printer")
    {
      THEN("Should print (- 10)")
      {
        ast::UnaryNegateExpr expr{ast::LiteralExpr::create(eml::Value{10.})};

        REQUIRE(eml::string_from_ast(expr) == "(- 10)");
      }
    }
  }

  GIVEN("A binary arithmatic expression 3 * (4 + 5) / (-3 - 1)")
  {
    const ast::Expr_ptr expr = ast::DivOpExpr::create(
        ast::MultOpExpr::create(
            ast::LiteralExpr::create(eml::Value{3.}),
            ast::PlusOpExpr::create(ast::LiteralExpr::create(eml::Value{4.}),
                                    ast::LiteralExpr::create(eml::Value{5.}))),
        ast::MinusOpExpr::create(ast::UnaryNegateExpr::create(
                                     ast::LiteralExpr::create(eml::Value{3.})),
                                 ast::LiteralExpr::create(eml::Value{1.})));
    WHEN("Invoke the Ast Printer")
    {
      const auto str = eml::string_from_ast(*expr);
      THEN("Should print the correct prefix notation")
      {
        REQUIRE(str == "(/ (* 3 (+ 4 5)) (- (- 3) 1))");
      }
    }

    WHEN("Compile into Bytecode")
    {
      eml::VM vm{};
      eml::Compiler compiler{};
      const auto c = compiler.bytecode_from_ast(*expr);
      THEN("Should produces the expected instruction sets")
      {
        eml::Bytecode expected;
        push_number(expected, 3.);
        push_number(expected, 4.);
        push_number(expected, 5.);
        write_instruction(expected, eml::op_add_f64);
        write_instruction(expected, eml::op_multiply_f64);
        push_number(expected, 3.);
        write_instruction(expected, eml::op_negate_f64);
        push_number(expected, 1.);
        write_instruction(expected, eml::op_subtract_f64);
        write_instruction(expected, eml::op_divide_f64);

        REQUIRE(c.disassemble() == expected.disassemble());
      }

      THEN("Evaluate to -6.75")
      {
        const auto result = vm.interpret(c);
        REQUIRE(result);
        REQUIRE(result->is_number());
        REQUIRE(result->unsafe_as_number() == Approx(-6.75));
      }
    }
  }
}
