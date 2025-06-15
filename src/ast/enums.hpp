#ifndef HULK_AST_ENUMS_HPP
#define HULK_AST_ENUMS_HPP 1

#include "../lexer/tokens"

namespace hulk {
namespace ast {
enum class unary_op {
  NOT = static_cast<int>(lexer::token_type::OP_NOT),
  MINUS = static_cast<int>(lexer::token_type::OP_MINUS)
};

enum class binary_op {
  PLUS = static_cast<int>(lexer::token_type::OP_PLUS),
  MINUS = static_cast<int>(lexer::token_type::OP_MINUS),
  DIVIDE = static_cast<int>(lexer::token_type::OP_DIVIDE),
  MULT = static_cast<int>(lexer::token_type::OP_MULTIPLY),
  MODULE = static_cast<int>(lexer::token_type::OP_MODULE),
  EXPONENT = static_cast<int>(lexer::token_type::OP_EXPONENT),
  GREATER = static_cast<int>(lexer::token_type::OP_GREATER),
  GREATER_EQUAL = static_cast<int>(lexer::token_type::OP_GREATER_EQ),
  LESS = static_cast<int>(lexer::token_type::OP_LESS),
  LESS_EQUAL = static_cast<int>(lexer::token_type::OP_LESS_EQ),
  NOT_EQUAL = static_cast<int>(lexer::token_type::OP_NOT_EQUAL),
  EQUAL_EQUAL = static_cast<int>(lexer::token_type::OP_EQUAL),
  OR = static_cast<int>(lexer::token_type::OP_OR),
  AND = static_cast<int>(lexer::token_type::OP_AND),
  CONCAT = static_cast<int>(lexer::token_type::OP_CONCAT),
  CONCAT_DOBLE = static_cast<int>(lexer::token_type::OP_DOBLE_CONCAT)
};

enum class function_type {
  NONE,
  FUNCTION,
  METHOD,
};
}  // namespace ast
}  // namespace hulk

#endif