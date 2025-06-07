#ifndef HULK_AST_HPP
#define HULK_AST_HPP 1

#include <memory>
#include <variant>

#include "../internal/internal_uncopyable"
#include "../lexer/tokens"

namespace hulk {

namespace ast {

enum class unary_op {
  NOT = lexer::token_type::OP_NOT,
  MINUS = lexer::token_type::OP_MINUS
};

enum class binary_op {
  PLUS = lexer::token_type::OP_PLUS,
  MINUS = lexer::token_type::OP_MINUS,
  DIVIDE = lexer::token_type::OP_DIVIDE,
  MULT = lexer::token_type::OP_MULTIPLY,
  GREATER = lexer::token_type::OP_GREATER,
  GREATER_EQUAL = lexer::token_type::OP_GREATER_EQ,
  LESS = lexer::token_type::OP_LESS,
  LESS_EQUAL = lexer::token_type::OP_LESS_EQ,
  NOT_EQUAL = lexer::token_type::OP_NOT_EQUAL,
  EQUAL_EQUAL = lexer::token_type::OP_EQUAL,
};

enum class logical_op {
  OR = lexer::token_type::OP_OR,
  AND = lexer::token_type::OP_AND
};

enum class right_left_op { EXP = lexer::token_type::OP_EXPONENT };

struct binary_expr;
struct call_expr;
struct grouping_expr;
struct literal_expr;
struct logical_expr;
struct unary_expr;
struct var_expr;
struct assign_expr;
struct right_left_expr;

using binary_expr_ptr = std::unique_ptr<binary_expr>;
using call_expr_ptr = std::unique_ptr<call_expr>;
using grouping_expr = std::unique_ptr<grouping_expr>;
using literal_expr = std::unique_ptr<literal_expr>;
using logical_expr_ptr = std::unique_ptr<logical_expr>;
using unary_expr_ptr = std::unique_ptr<unary_expr>;
using var_expr_ptr = std::unique_ptr<var_expr>;
using assign_expr_ptr = std::unique_ptr<assign_expr>;
using right_left_expr_ptr = std::unique_ptr<right_left_expr>;

using expr = std::variant<
    binary_expr_ptr, call_expr_ptr, grouping_expr, literal_expr, logical_expr_ptr, unary_expr_ptr, var_expr_ptr, assign_expr_ptr, right_left_expr_ptr, >;

struct assignable : private internal::uncopyable {
  lexer::token name;
  mutable int distance = -1;
  mutable bool is_captured = false;
  explicit assignable(const lexer::token &_name) : name(_name) {}
};

struct binary_expr final : private internal::uncopyable {
  expr left;
  lexer::token token;
  binary_op op;
  expr right;
  explicit binary_expr(expr _left, const lexer::token &_token,
                       const binary_op _op, expr _right)
      : left(std::move(_left)),
        token(_token),
        op(_op),
        right(std::move(_right)) {}
};

struct call_expr : private internal::uncopyable {
  expr calle;
  lexer::token keyword;
  std::vector<expr> arguments;
  explicit call_expr(expr _calle, const lexer::token &_keyword,
                     std::vector<expr> _arguments)
      : calle(std::move(_calle)),
        keyword(_keyword),
        arguments(std::move(_arguments)) {}
};

struct unary_expr : internal::uncopyable {
  lexer::token token;
  unary_op op;
  expr expression;
  explicit unary_expr(const lexer::token &_token, const unary_op _op,
                      expr _expression)
      : token(_token), op(_op), expression(std::move(_expression)) {}
};

struct grouping_expr : internal::uncopyable {
  expr expression;
  explicit grouping_expr(expr _expression) : expression(std::move(_expression)) {}
};

struct literal_expr : internal::uncopyable {
  lexer::literal value;
  explicit literal_expr(const lexer::literal& _value) : value(_value) {}
};

}  // namespace ast

}  // namespace hulk

#endif  // HULK_AST_HPP