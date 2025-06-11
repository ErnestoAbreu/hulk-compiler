#ifndef HULK_AST_HPP
#define HULK_AST_HPP 1

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "../internal/internal_uncopyable"
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
  GREATER = static_cast<int>(lexer::token_type::OP_GREATER),
  GREATER_EQUAL = static_cast<int>(lexer::token_type::OP_GREATER_EQ),
  LESS = static_cast<int>(lexer::token_type::OP_LESS),
  LESS_EQUAL = static_cast<int>(lexer::token_type::OP_LESS_EQ),
  NOT_EQUAL = static_cast<int>(lexer::token_type::OP_NOT_EQUAL),
  EQUAL_EQUAL = static_cast<int>(lexer::token_type::OP_EQUAL),
};

enum class logical_op {
  OR = static_cast<int>(lexer::token_type::OP_OR),
  AND = static_cast<int>(lexer::token_type::OP_AND)
};

enum class function_type {
  NONE,
  FUNCTION,
  METHOD,
};

struct binary_expr;
struct call_expr;
struct get_expr;
struct set_expr;
struct this_expr;
struct literal_expr;
struct logical_expr;
struct unary_expr;
struct var_expr;
struct assign_expr;
struct grouping_expr;
struct let_expr;
struct while_expr;
struct if_expr;
struct var_expr;
struct block_expr;

using this_expr_ptr = std::unique_ptr<this_expr>;
using binary_expr_ptr = std::unique_ptr<binary_expr>;
using call_expr_ptr = std::unique_ptr<call_expr>;
using get_expr_ptr = std::unique_ptr<get_expr>;
using set_expr_ptr = std::unique_ptr<set_expr>;
using literal_expr_ptr = std::unique_ptr<literal_expr>;
using logical_expr_ptr = std::unique_ptr<logical_expr>;
using unary_expr_ptr = std::unique_ptr<unary_expr>;
using var_expr_ptr = std::unique_ptr<var_expr>;
using assign_expr_ptr = std::unique_ptr<assign_expr>;
using grouping_expr_ptr = std::unique_ptr<grouping_expr>;
using let_expr_ptr = std::unique_ptr<let_expr>;
using while_expr_ptr = std::unique_ptr<while_expr>;
using var_expr_ptr = std::unique_ptr<var_expr>;
using if_expr_ptr = std::unique_ptr<if_expr>;
using block_expr_ptr = std::unique_ptr<block_expr>;

using expr = std::variant<this_expr_ptr, binary_expr_ptr, call_expr_ptr,
                          get_expr_ptr, set_expr_ptr, literal_expr_ptr,
                          logical_expr_ptr, unary_expr_ptr, var_expr_ptr,
                          assign_expr_ptr, let_expr_ptr, while_expr_ptr,
                          if_expr_ptr, block_expr_ptr, grouping_expr_ptr>;

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

struct get_expr : private internal::uncopyable {
  expr object;
  lexer::token name;
  explicit get_expr(expr _object, const lexer::token &_name)
      : object(std::move(_object)), name(_name) {}
};

struct set_expr : private internal::uncopyable {
  expr object;
  lexer::token name;
  expr value;
  explicit set_expr(expr _object, const lexer::token &_name, expr _value)
      : object(std::move(_object)), name(_name), value(std::move(_value)) {}
};

struct this_expr : assignable {
  explicit this_expr(const lexer::token &_name) : assignable(_name) {}
};

struct super_expr : assignable {
  lexer::token method;
  explicit super_expr(const lexer::token &_name, const lexer::token &_method)
      : assignable(_name), method(_method) {}
};

struct unary_expr : private internal::uncopyable {
  lexer::token token;
  unary_op op;
  expr expression;
  explicit unary_expr(const lexer::token &_token, const unary_op _op,
                      expr _expression)
      : token(_token), op(_op), expression(std::move(_expression)) {}
};

struct grouping_expr : internal::uncopyable {
  expr expression;
  explicit grouping_expr(expr _expression)
      : expression(std::move(_expression)) {}
};

struct literal_expr : private internal::uncopyable {
  lexer::literal value;
  explicit literal_expr(const lexer::literal &_value) : value(_value) {}
};

struct logical_expr : private internal::uncopyable {
  expr left;
  logical_op op;
  expr right;
  logical_expr(expr _left, logical_op _op, expr _right)
      : left(std::move(_left)), op(_op), right(std::move(_right)) {}
};

struct var_expr : assignable {
  explicit var_expr(const lexer::token &_name) : assignable(_name) {}
};

struct assign_expr : assignable {
  expr value;
  explicit assign_expr(const lexer::token &_name, expr _value)
      : assignable(_name), value(std::move(_value)) {}
};

struct if_expr : private internal::uncopyable {
  expr condition;
  expr then_branch;
  std::vector<std::pair<expr, expr>> elif_branchs;
  std::optional<expr> else_branch;
  explicit if_expr(expr _condition, expr _then_branch,
                   std::vector<std::pair<expr, expr>> _elif_branchs,
                   std::optional<expr> _else_branch)
      : condition(std::move(_condition)),
        then_branch(std::move(_then_branch)),
        elif_branchs(std::move(_elif_branchs)),
        else_branch(std::move(_else_branch)) {}
};

struct block_expr : private internal::uncopyable {
  std::vector<expr> expressions;
  explicit block_expr(std::vector<expr> _expressions)
      : expressions(std::move(_expressions)) {}
};

struct let_expr : private internal::uncopyable {
  std::vector<assign_expr_ptr> assignments;
  expr body;
  explicit let_expr(std::vector<assign_expr_ptr> _assignments, expr _body)
      : assignments(std::move(_assignments)), body(std::move(_body)) {}
};

struct while_expr : private internal::uncopyable {
  expr condition;
  expr body;
  while_expr(expr _condition, expr _body)
      : condition(std::move(_condition)), body(std::move(_body)) {}
};

struct expression_stmt;
struct function_stmt;
struct class_stmt;
struct field_stmt;
struct super_item;

using expression_stmt_ptr = std::shared_ptr<expression_stmt>;
using function_stmt_ptr = std::shared_ptr<function_stmt>;
using class_stmt_ptr = std::shared_ptr<class_stmt>;
using field_stmt_ptr = std::shared_ptr<field_stmt>;
using super_item_ptr = std::shared_ptr<super_item>;

using stmt = std::variant<expression_stmt_ptr, function_stmt_ptr,
                          class_stmt_ptr, field_stmt_ptr, super_item_ptr>;

using stmt_list = std::vector<stmt>;

struct parameter {
  lexer::token name;
  lexer::token type;
  parameter(lexer::token _name, lexer::token _type)
      : name(_name), type(_type) {}
};

struct expression_stmt : private internal::uncopyable {
  expr expression;
  explicit expression_stmt(expr _expression)
      : expression(std::move(_expression)) {}
};

struct function_stmt : private internal::uncopyable {
  lexer::token name;
  function_type type;
  std::vector<parameter> parameters;
  expr body;
  explicit function_stmt(const lexer::token &_name, const function_type _type,
                         std::vector<parameter> _parameters, expr _body)
      : name(_name),
        type(_type),
        parameters(std::move(_parameters)),
        body(std::move(_body)) {}
};

struct field_stmt : private internal::uncopyable {
  lexer::token name;
  expr initializer;
  explicit field_stmt(const lexer::token &_name, expr _initializer)
      : name(_name), initializer(std::move(_initializer)) {}
};

struct super_item : internal::uncopyable {
  lexer::token name;
  std::vector<expr> init;
  explicit super_item(const lexer::token &_name, std::vector<expr> _init)
      : name(_name), init(std::move(_init)) {}
};

struct class_stmt : private internal::uncopyable {
  lexer::token name;
  std::vector<parameter> parameters;
  std::optional<super_item_ptr> super_class;
  std::vector<field_stmt_ptr> fields;
  std::vector<function_stmt_ptr> methods;

  explicit class_stmt(const lexer::token &_name,
                      std::optional<super_item_ptr> _super,
                      std::vector<field_stmt_ptr> _fields,
                      std::vector<function_stmt_ptr> _methods)
      : name(_name),
        super_class(std::move(_super)),
        fields(std::move(_fields)),
        methods(std::move(_methods)) {}
};

using program = std::vector<stmt>;

}  // namespace ast

}  // namespace hulk

#endif  // HULK_AST_HPP