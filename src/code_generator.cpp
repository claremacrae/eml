#include "ast.hpp"
#include "compiler.hpp"

namespace eml {

namespace {

struct CodeGenerator;

// Emit different push instructions depends on they of an expression
struct TypeDispatcher {
  CodeGenerator& generator;
  Value v;

  void operator()(const NumberType& /*t*/);

  void operator()(const BoolType& /*t*/);

  void operator()(const UnitType& /*t*/);

  void operator()(const StringType& /*t*/);

  [[noreturn]] void operator()(const ErrorType& /*t*/);
};

struct CodeGenerator : AstConstVisitor {
  friend TypeDispatcher;

  explicit CodeGenerator(Bytecode& chunk, const Compiler& compiler)
      : chunk_{chunk}, compiler_{compiler}
  {
  }

  void operator()(const LiteralExpr& constant) override
  {
    TypeDispatcher visitor{*this, constant.value()};
    std::visit(visitor, constant.type());
  }

  void operator()([[maybe_unused]] const IdentifierExpr& id) override
  {
    EML_ASSERT(id.value() != std::nullopt,
               "Identifier expression passed to the code generator are "
               "garanteed to have a value");
    TypeDispatcher visitor{*this, *id.value()};
    std::visit(visitor, id.type());
  }

  void unary_common(const UnaryOpExpr& expr, opcode op)
  {
    expr.operand().accept(*this);
    chunk_.write(op, line_num{0});
  }

  void operator()(const UnaryNegateExpr& expr) override
  {
    unary_common(expr, op_negate_f64);
  }

  void operator()(const UnaryNotExpr& expr) override
  {
    unary_common(expr, op_not);
  }

  void binary_common(const BinaryOpExpr& expr, opcode op)
  {
    expr.lhs().accept(*this);
    expr.rhs().accept(*this);
    chunk_.write(op, line_num{0});
  }

  void operator()(const PlusOpExpr& expr) override
  {
    binary_common(expr, op_add_f64);
  }
  void operator()(const MinusOpExpr& expr) override
  {
    binary_common(expr, op_subtract_f64);
  }
  void operator()(const MultOpExpr& expr) override
  {
    binary_common(expr, op_multiply_f64);
  }
  void operator()(const DivOpExpr& expr) override
  {
    binary_common(expr, op_divide_f64);
  }
  void operator()(const EqOpExpr& expr) override
  {
    binary_common(expr, op_equal);
  }
  void operator()(const NeqOpExpr& expr) override
  {
    binary_common(expr, op_not_equal);
  }
  void operator()(const LessOpExpr& expr) override
  {
    binary_common(expr, op_less_f64);
  }
  void operator()(const LeOpExpr& expr) override
  {
    binary_common(expr, op_less_equal_f64);
  }
  void operator()(const GreaterOpExpr& expr) override
  {
    binary_common(expr, op_greater_f64);
  }
  void operator()(const GeExpr& expr) override
  {
    binary_common(expr, op_greater_equal_f64);
  }

  void operator()(const LambdaExpr& /*expr*/) override
  {
    throw "TODO";
  }

  // Emits [instruction] followed by a placeholder for a jump offset. The
  // placeholder can be patched by calling [jumpPatch]. Returns the index of the
  // placeholder.
  auto write_jump(eml::opcode jump_instruction, line_num linum)
      -> std::ptrdiff_t
  {
    chunk_.write(jump_instruction, linum);
    const auto jump = chunk_.write(std::byte{}, linum);
    return jump;
  }

  // Replaces the placeholder argument for a previous jump
  // instruction with an offset that jumps to the current end of bytecode.
  void jump_patch(std::ptrdiff_t index)
  {
    const auto jump_to = chunk_.next_instruction_index();
    chunk_.write_at(static_cast<std::byte>(jump_to - index - 1), index);
  }

  void operator()(const IfExpr& expr) override
  {
    EML_ASSERT(eml::match(expr.cond().type(), BoolType{}),
               "Type of condition must be boolean");
    EML_ASSERT(eml::match(expr.If().type(), expr.Else().type()),
               "Type of different branches must match");

    expr.cond().accept(*this);
    const auto else_jump_pos = write_jump(eml::op_jmp_false, line_num{0});

    expr.If().accept(*this);

    const auto if_jump_pos = write_jump(eml::op_jmp, line_num{0});

    jump_patch(else_jump_pos);

    expr.Else().accept(*this);

    jump_patch(if_jump_pos);
  }

  void operator()(const Definition& /*def*/) override {} // no-op

  Bytecode& chunk_; // Not null
  const Compiler& compiler_;
};

void TypeDispatcher::operator()(const NumberType&)
{
  const auto offset = generator.chunk_.add_constant(v);

  generator.chunk_.write(eml::op_push_f64, line_num{0});
  generator.chunk_.write(std::byte{*offset}, line_num{0});
}

void TypeDispatcher::operator()(const StringType&)
{
  const auto offset = generator.chunk_.add_constant(v);

  generator.chunk_.write(eml::op_push_f64, line_num{0});
  generator.chunk_.write(std::byte{*offset}, line_num{0});
}

void TypeDispatcher::operator()(const BoolType&)
{
  if (v.unsafe_as_boolean()) {
    generator.chunk_.write(eml::op_true, line_num{0});
  } else {
    generator.chunk_.write(eml::op_false, line_num{0});
  }
}

void TypeDispatcher::operator()(const UnitType&)
{
  generator.chunk_.write(eml::op_unit, line_num{0});
}

void TypeDispatcher::operator()(const ErrorType& /*t*/)
{
  EML_UNREACHABLE();
}

} // anonymous namespace

auto Compiler::generate_code(const AstNode& expr) const
    -> std::tuple<Bytecode, Type>
{
  Bytecode code;
  CodeGenerator code_generator{code, *this};
  expr.accept(code_generator);
  return std::tuple(code, expr.type());
}

} // namespace eml
