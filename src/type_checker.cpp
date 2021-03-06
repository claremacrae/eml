#include "ast.hpp"
#include "compiler.hpp"

#include <iomanip>
#include <sstream>

#include "debug.hpp"

namespace eml {

struct Func1Type {
  Type arg_type;
  Type result_type;
};

struct Func2Type {
  Type arg1_type;
  Type arg2_type;
  Type result_type;
};

namespace {
struct TypeChecker : AstVisitor {
  Compiler& compiler;
  bool has_error = false;
  bool panic_mode = false;
  std::vector<CompilationError> errors;

  explicit TypeChecker(Compiler& c) : compiler(c) {}

  void operator()([[maybe_unused]] LiteralExpr& constant) override
  {
    // no-op
    EML_ASSERT(constant.has_type(),
               "All literal should have a type assigned from the parser");
  }

  void operator()(IdentifierExpr& id) override
  {
    const auto query_result = compiler.get_global(id.name());
    if (query_result) {
      id.set_type(query_result->first);
      id.set_value(query_result->second);
    } else {
      id.set_type(ErrorType{});
      std::stringstream ss;
      ss << "Undefined identifier: " << id.name() << '\n';
      error(ss.str());
    }
  }

  void unary_common(UnaryOpExpr& expr, std::string_view op,
                    const Func1Type& allowed_type)
  {
    expr.operand().accept(*this);
    if (match(expr.operand().type(), allowed_type.arg_type)) {
      expr.set_type(allowed_type.result_type);
    } else {
      expr.set_type(ErrorType{});
      if (!panic_mode) {
        std::stringstream ss;
        const auto align = 8;
        ss << "Unmatched types around of unary operator " << op << '\n';
        ss << std::left << "Requires " << op << " " << std::setw(align)
           << allowed_type.arg_type << '\n';
        ss << std::left << "Has      " << op << " " << std::setw(align)
           << expr.operand().type() << '\n';
        error(ss.str());
      }
    }
  }

  void operator()(UnaryNegateExpr& expr) override
  {
    unary_common(expr, "-", Func1Type{NumberType{}, NumberType{}});
  }

  void operator()(UnaryNotExpr& expr) override
  {
    unary_common(expr, "!", Func1Type{BoolType{}, BoolType{}});
  }

  void error(const std::string& message)
  {
    if (panic_mode) {
      return;
    }
    has_error = true;
    panic_mode = true;

    errors.emplace_back(std::in_place_type<TypeError>, message);
  }

  void binary_common(BinaryOpExpr& expr, std::string_view op,
                     const Func2Type& allowed_type)
  {
    expr.lhs().accept(*this);
    expr.rhs().accept(*this);
    if (eml::match(expr.lhs().type(), allowed_type.arg1_type) &&
        eml::match(expr.rhs().type(), allowed_type.arg2_type)) {
      expr.set_type(allowed_type.result_type);
    } else {
      expr.set_type(ErrorType{});
      if (!panic_mode) {
        const auto align = 8;
        std::stringstream ss;
        ss << "Unmatched types around binary operator " << op << '\n';
        ss << std::left << "Requires " << std::setw(align)
           << allowed_type.arg1_type << std::setw(3) << op << std::setw(align)
           << allowed_type.arg2_type << '\n';
        ss << "Has      " << std::setw(align) << expr.lhs().type()
           << std::setw(3) << op << std::setw(align) << expr.rhs().type()
           << '\n';
        error(ss.str());
      }
    }
  }

  void equality_common(BinaryOpExpr& expr, std::string_view op)
  {
    expr.lhs().accept(*this);
    expr.rhs().accept(*this);
    if (eml::match(expr.lhs().type(), expr.rhs().type())) {
      expr.set_type(BoolType{});
    } else {
      expr.set_type(ErrorType{});
      if (!panic_mode) {
        std::stringstream ss;
        ss << "Unmatched types around comparison operator " << op << '\n';
        ss << "Requires "
           << "T " << op << " T\n";
        ss << "where T: EqualityComparable\n";
        ss << "Has " << expr.lhs().type() << " " << op << " "
           << expr.rhs().type() << '\n';
        error(ss.str());
      }
    }
  }

  void operator()(PlusOpExpr& expr) override
  {
    binary_common(expr, "+",
                  Func2Type{NumberType{}, NumberType{}, NumberType{}});
  }
  void operator()(MinusOpExpr& expr) override
  {
    binary_common(expr, "-",
                  Func2Type{NumberType{}, NumberType{}, NumberType{}});
  }
  void operator()(MultOpExpr& expr) override
  {
    binary_common(expr, "*",
                  Func2Type{NumberType{}, NumberType{}, NumberType{}});
  }
  void operator()(DivOpExpr& expr) override
  {
    binary_common(expr, "/",
                  Func2Type{NumberType{}, NumberType{}, NumberType{}});
  }
  void operator()(EqOpExpr& expr) override
  {
    equality_common(expr, "==");
  }
  void operator()(NeqOpExpr& expr) override
  {
    equality_common(expr, "!=");
  }
  void operator()(LessOpExpr& expr) override
  {
    binary_common(expr, "<", Func2Type{NumberType{}, NumberType{}, BoolType{}});
  }
  void operator()(LeOpExpr& expr) override
  {
    binary_common(expr,
                  "<=", Func2Type{NumberType{}, NumberType{}, BoolType{}});
  }
  void operator()(GreaterOpExpr& expr) override
  {
    binary_common(expr, ">", Func2Type{NumberType{}, NumberType{}, BoolType{}});
  }
  void operator()(GeExpr& expr) override
  {
    binary_common(expr,
                  ">=", Func2Type{NumberType{}, NumberType{}, BoolType{}});
  }

  void operator()(IfExpr& expr) override
  {

    expr.cond().accept(*this);
    expr.If().accept(*this);
    expr.Else().accept(*this);

    if (!eml::match(expr.cond().type(), BoolType{})) {
      expr.set_type(ErrorType{});
      if (!panic_mode) {
        std::stringstream ss;
        ss << "I want a " << BoolType{} << " in condition of if expression\n";
        ss << "Got " << expr.cond().type() << '\n';
        error(ss.str());
      }
    } else if (!eml::match(expr.If().type(), expr.Else().type())) {
      expr.set_type(ErrorType{});
      std::stringstream ss;
      ss << "Type mismatch in branching!\n";
      ss << "If branch: " << expr.If().type() << '\n';
      ss << "Else branch: " << expr.Else().type() << '\n';
      error(ss.str());
    } else {
      expr.set_type(expr.If().type());
    }
  }

  void operator()(LambdaExpr& expr) override
  {
    error("Functions are not implemented yet!");
    expr.set_type(ErrorType{});
  }

  void operator()(Definition& def) override
  {
    def.to().accept(*this);

    if (def.binding_type().has_value()) {
      if (!match(*def.binding_type(), def.to().type())) {
        std::stringstream ss;
        ss << "Type mismatch in value definition\n";
        ss << "Got let" << *def.binding_type() << " = " << def.to().type();
        error(ss.str());
      }
    } else {
      def.set_binding_type(def.to().type());
    }

    // TODO(Lesley Lai): implement constant folding
    const auto v = dynamic_cast<const LiteralExpr*>(&def.to());

    if (v == nullptr) {
      error("Constant folding is unimplemented yet");
    } else {
      compiler.add_global(std::string{def.identifier()}, *def.binding_type(),
                          v->value());
    }
  }

}; // namespace
} // anonymous namespace

Compiler::TypeCheckResult Compiler::type_check(std::unique_ptr<AstNode>& ptr)
{
  TypeChecker type_checker{*this};
  ptr->accept(type_checker);
  if (!type_checker.has_error) {
    return std::move(ptr);
  } else {
    return unexpected{std::move(type_checker.errors)};
  }
}

} // namespace eml
