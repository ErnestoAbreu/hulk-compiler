#ifndef HULK_PARSER_H
#define HULK_PARSER_H 1

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "../ast/ast"
#include "../ast/enums"
#include "../internal/internal_error"
#include "../lexer/tokens"

namespace hulk {

namespace parser {

struct parse_error final : public std::runtime_error {
  explicit parse_error(const std::string &message) : runtime_error(message) {}
};

struct parser {
  explicit parser(const std::vector<lexer::token> &_tokens) : tokens(_tokens) {}

  ast::program parse() {
    std::vector<ast::stmt_ptr> decl_list;
    ast::expr_ptr main_expr;
    try {
      bool found_expr = false;
      while (!is_at_end()) {
        if (auto decl = declaration(); decl.has_value()) {
          decl_list.push_back(std::move(decl.value()));
        }

        bool found_semicolon = false;
        if (match(TT::SEMICOLON)) found_semicolon = true;
      
        if (!decl_list.empty()) {
          auto stmt = decl_list.back();
          if (ast::expression_stmt *t = dynamic_cast<ast::expression_stmt *>(stmt.get())) {
            if (found_expr) {
              error(previous(), "A program in HULK can consist of just one global expression.");
              break;
            }

            if (!found_semicolon && previous().get_type() != TT::RBRACE) {
              error(peek(), "Expected ';' after main expression.");
              break;
            }
            
            main_expr = std::move(t->expression);
            decl_list.pop_back();
            found_expr = true;
          }
        }
      }
    } catch (const std::invalid_argument &e) {
      std::cout << "error: " << e.what() << std::endl;
    }
    return ast::program(std::move(decl_list), std::move(main_expr));
  }

 private:
  std::vector<lexer::token> tokens;

  int current = 0;

  using TT = lexer::token_type;

  using parserFn = ast::expr_ptr (parser::*)();

  std::optional<ast::stmt_ptr> declaration() {
    try {
      if (match(TT::KW_FUNCTION)) return function(ast::function_type::FUNCTION);
      if (match(TT::KW_TYPE)) return class_declaration();
      if (match(TT::KW_PROTOCOL)) return protocol_declaration();

      return statement();
    } catch (parse_error &) {
      sync();
      return std::make_optional<ast::stmt_ptr>();
    }
  }

  lexer::token opt_type(bool is_optional = true) {
    lexer::token result;
    if (match(TT::COLON)) {
      result = consume(TT::IDENTIFIER, "Expected a type after ':' in definition.");
    } else if (!is_optional) {
      error(peek(), "Expected type.");
    }
    return result;
  }

  ast::protocol_stmt_ptr protocol_declaration() {
    auto name = consume(TT::IDENTIFIER, "Expected protocol name.");

    lexer::token super_protocol;
    if (match(TT::KW_EXTENDS)) {
      super_protocol =
          consume(TT::IDENTIFIER, "Expected super protocol extends name.");
    }

    consume(TT::LBRACE, "Expected '{' before protocol signatures.");

    std::vector<ast::function_stmt_ptr> methods;
    while (!check(TT::RBRACE) && !is_at_end()) {
      auto name = consume(TT::IDENTIFIER, "Expected method name.");

      consume(TT::LPAREN, "Expected '(' before parameters.");

      std::vector<ast::parameter> parameters;
      if (!check(TT::RPAREN)) {
        do {
          auto param_name = consume(TT::IDENTIFIER, "Expected parameter name.");
          auto param_type = opt_type(false);
          parameters.push_back(ast::parameter(param_name, param_type));
        } while (match(TT::COMMA));
      }

      consume(TT::RPAREN, "Expected ')' after parameters.");

      auto return_type = opt_type(false);
      methods.push_back(std::make_shared<ast::function_stmt>(
          name, ast::function_type::METHOD, std::move(parameters), nullptr,
          return_type));

      consume(TT::SEMICOLON, "Expected ';' after protocol method declaration.");
    }

    consume(TT::RBRACE, "Expected '}' after protocol signatures.");
    return std::make_shared<ast::protocol_stmt>(name, std::move(methods),
                                                super_protocol);
  }

  ast::class_stmt_ptr class_declaration() {
    auto name = consume(TT::IDENTIFIER, "Expected type name.");

    std::vector<ast::parameter> parameters;
    if (match(TT::LPAREN)) {
      if (!check(TT::RPAREN)) {
        do {
          lexer::token param_name = consume(TT::IDENTIFIER, "Expected parameter name.");
          lexer::token param_type = opt_type();

          parameters.push_back(ast::parameter(param_name, param_type));
        } while (match(TT::COMMA));
      }

      consume(TT::RPAREN, "Expected ')' after parameters");
    }

    std::optional<ast::super_item_ptr> super_class;
    if (match(TT::KW_INHERITS)) {
      lexer::token super_name =
          consume(TT::IDENTIFIER, "Expected super_class name.");

      std::vector<ast::expr_ptr> args;
      if (match(TT::LPAREN)) {
        do {
          auto expr = expression();
          args.push_back(std::move(expr));
        } while (match(TT::COMMA));

        consume(TT::RPAREN, "Expected ')' after superclass arguments.");
      }

      super_class =
          std::make_unique<ast::super_item>(super_name, std::move(args));
    }

    consume(TT::LBRACE, "Expected '{' before type body.");

    std::vector<ast::field_stmt_ptr> fields;
    std::vector<ast::function_stmt_ptr> methods;
    while (!check(TT::RBRACE) && !is_at_end()) {
      auto name = consume(TT::IDENTIFIER, "Expected field or method name.");

      if (check(TT::LPAREN)) {
        methods.push_back(function(ast::function_type::METHOD));
      } else {
        fields.push_back(field_declaration(name));
      }
    }

    consume(TT::RBRACE, "Expected '}' after type body.");

    return std::make_shared<ast::class_stmt>(name, parameters, std::move(super_class), std::move(fields), std::move(methods));
  }

  ast::field_stmt_ptr field_declaration(const lexer::token name) {
    lexer::token type = opt_type();
    ast::expr_ptr init = match(TT::OP_ASSIGN)
                             ? expression()
                             : std::make_unique<ast::literal_expr>(nullptr);
    consume(TT::SEMICOLON, "Expected ';' after field declaration.");
    return std::make_shared<ast::field_stmt>(name, type, std::move(init));
  }

  ast::while_expr_ptr while_expression() {
    consume(TT::LPAREN, "Expected '(' after while.");
    ast::expr_ptr condition = expression();
    consume(TT::RPAREN, "Expected ')' after condition.");
    ast::expr_ptr body = expression();
    return std::make_unique<ast::while_expr>(std::move(condition),
                                             std::move(body));
  }

  ast::stmt_ptr statement() { return expression_statement(); }

  ast::if_expr_ptr if_expression() {
    consume(TT::LPAREN, "Expected '(' after 'if'.");
    auto condition = expression();
    consume(TT::RPAREN, "Expected ')' after if condition.");

    auto then_branch = expression();

    std::vector<std::pair<ast::expr_ptr, ast::expr_ptr>> elif_branchs;
    while (match(TT::KW_ELIF)) {
      consume(TT::LPAREN, "Expected '(' after 'elif'.");
      auto condition = expression();
      consume(TT::RPAREN, "Expected ')' after elif condition");

      ast::expr_ptr cur_then_branch = expression();
      elif_branchs.emplace_back(std::move(condition),
                                std::move(cur_then_branch));
    }

    std::optional<ast::expr_ptr> else_branch = {};
    if (match(TT::KW_ELSE)) {
      else_branch = expression();
    }

    return std::make_unique<ast::if_expr>(
        std::move(condition), std::move(then_branch), std::move(elif_branchs),
        std::move(else_branch));
  }

  ast::expression_stmt_ptr expression_statement() {
    ast::expr_ptr expr = expression();
    return std::make_shared<ast::expression_stmt>(std::move(expr));
  }

  ast::function_stmt_ptr function(const ast::function_type type) {
    const std::string kind =
        type == ast::function_type::FUNCTION ? "function" : "method";

    lexer::token name = previous();
    if (kind == "function") {
      name = consume(TT::IDENTIFIER, "Expected " + kind + " name.");
    }

    consume(TT::LPAREN, "Expected '(' after " + kind + " name.");

    std::vector<ast::parameter> parameters;
    if (!check(TT::RPAREN)) {
      do {
        lexer::token param_name =
            consume(TT::IDENTIFIER, "Expected parameter name.");
        lexer::token param_type = opt_type();
        
        parameters.push_back(ast::parameter(param_name, param_type));
      } while (match(TT::COMMA) && !is_at_end());
    }

    consume(TT::RPAREN, "Expected ')' after parameters.");

    lexer::token return_type = opt_type();

    bool is_inline_fun = match(TT::ARROW);

    ast::expr_ptr body;
    if (is_inline_fun) {
      body = expression();
      consume(TT::SEMICOLON, "Expected ';' after inline " + kind + " body.");
    } else {
      consume(TT::LBRACE, "Expected '{' before full-form " + kind + "body.");
      body = block_expression();
    }

    return std::make_shared<ast::function_stmt>(name, type, parameters,
                                                std::move(body), return_type);
  }

  ast::block_expr_ptr block_expression() {
    std::vector<ast::expr_ptr> expressions;
    while (!check(TT::RBRACE) && !is_at_end()) {
      auto expr = expression();
      expressions.push_back(std::move(expr));
      consume(TT::SEMICOLON, "Expected ';' after expressions in block.");
    }
    consume(TT::RBRACE, "Expected '}' after block.");
    return std::make_unique<ast::block_expr>(std::move(expressions));
  }

  ast::expr_ptr parse_binary_expr(
      const std::initializer_list<lexer::token_type> &types, ast::expr_ptr expr,
      const parserFn &f) {
    while (match(types)) {
      auto token = previous();
      expr = std::make_unique<ast::binary_expr>(
          std::move(expr), token, static_cast<ast::binary_op>(token.get_type()),
          std::invoke(f, this));
    }
    return expr;
  }

  ast::expr_ptr expression() {
    if (match(TT::KW_LET)) return let_expression();
    if (match(TT::KW_IF)) return if_expression();
    if (match(TT::KW_WHILE)) return while_expression();
    if (match(TT::LBRACE)) return block_expression();
    if (match(TT::KW_FOR)) return for_expression();

    return assignment();
  }

  ast::for_expr_ptr for_expression() {
    consume(TT::LPAREN, "Expected '(' after for.");
    auto name = consume(TT::IDENTIFIER, "Expected variable name.");
    auto type = opt_type();
    consume(TT::KW_IN, "Expected 'in' after variable declaration.");
    ast::expr_ptr iter = expression();
    consume(TT::RPAREN, "Expected ')' after iterator expression.");
    ast::expr_ptr body = expression();
    return std::make_unique<ast::for_expr>(name, type, std::move(iter),
                                           std::move(body));
  }

  ast::let_expr_ptr let_expression() {
    std::vector<ast::declaration_expr_ptr> decl;
    do {
      const lexer::token name =
          consume(TT::IDENTIFIER, "Expected variable name.");
      auto type = opt_type();

      ast::expr_ptr init = match(TT::OP_ASSIGN)
                               ? expression()
                               : std::make_unique<ast::literal_expr>(nullptr);

      decl.push_back(
          std::make_unique<ast::declaration_expr>(name, type, std::move(init)));
    } while (match(TT::COMMA));

    consume(TT::KW_IN, "Expected 'in' after variable declarations in let-in.");

    ast::expr_ptr body = expression();

    return std::make_unique<ast::let_expr>(std::move(decl), std::move(body));
  }

  ast::expr_ptr assignment() {
    auto expr = or_expr();
    if (match(TT::OP_DESTRUCT_ASSIGN)) {
      const auto equals = previous();
      auto value = assignment();

      if (ast::var_expr *t = dynamic_cast<ast::var_expr *>(expr.get())) {
        lexer::token type;
        return std::make_unique<ast::assign_expr>(std::move(expr), type, std::move(value));
      }

      error(equals, "Invalid assignment target.");
    }
    return expr;
  }

  ast::expr_ptr or_expr() {
    ast::expr_ptr expr = and_expr();
    while (match(TT::OP_OR)) {
      auto token = previous();
      auto right = and_expr();

      expr = std::make_unique<ast::binary_expr>(
          std::move(expr), token, ast::binary_op::OR, std::move(right));
    }
    return expr;
  }

  ast::expr_ptr and_expr() {
    auto expr = equality();
    while (match(TT::OP_AND)) {
      auto token = previous();
      auto right = equality();
      expr = std::make_unique<ast::binary_expr>(
          std::move(expr), token, ast::binary_op::AND, std::move(right));
    }
    return expr;
  }

  ast::expr_ptr equality() {
    return parse_binary_expr({TT::OP_NOT_EQUAL, TT::OP_EQUAL}, comparison(),
                             &parser::comparison);
  }

  ast::expr_ptr comparison() {
    return parse_binary_expr(
        {TT::OP_GREATER, TT::OP_GREATER_EQ, TT::OP_LESS, TT::OP_LESS_EQ},
        term(), &parser::term);
  }

  ast::expr_ptr term() {
    return parse_binary_expr({TT::OP_MINUS, TT::OP_PLUS}, factor(),
                             &parser::factor);
  }

  ast::expr_ptr factor() {
    return parse_binary_expr(
        {TT::OP_DIVIDE, TT::OP_MODULE, TT::OP_MULTIPLY, TT::OP_EXPONENT,
         TT::OP_CONCAT, TT::OP_DOBLE_CONCAT},
        unary(), &parser::unary);
  }

  ast::expr_ptr unary() {
    if (match({TT::OP_NOT, TT::OP_MINUS})) {
      const auto token = previous();
      ast::expr_ptr right = unary();
      return std::make_unique<ast::unary_expr>(
          token, static_cast<ast::unary_op>(token.get_type()),
          std::move(right));
    }
    return var_call();
  }

  ast::expr_ptr var_call() {
    ast::expr_ptr expr = primary();
    
    while (match(TT::DOT)) {
        std::optional<ast::expr_ptr> object = std::move(expr);
        lexer::token name = consume(TT::IDENTIFIER, "Expected property name after '.'.");

        if(match(TT::LPAREN))
          expr = finish_call(object, name);
        else 
          expr = std::make_unique<ast::var_expr>(std::move(expr), name);
    }

    return expr;
  }

  ast::expr_ptr finish_call(std::optional<ast::expr_ptr> &object, lexer::token& calle) {
    std::vector<ast::expr_ptr> arguments;

    if (!check(TT::RPAREN)) {
      do {
        arguments.push_back(expression());
      } while (match(TT::COMMA));
    }

    consume(TT::RPAREN, "Expected ')' after arguments.");

    return std::make_unique<ast::call_expr>(std::move(object), calle, std::move(arguments));
  }

  ast::expr_ptr primary() {
    if (match(TT::FALSE)) return std::make_unique<ast::literal_expr>(false);
    if (match(TT::TRUE)) return std::make_unique<ast::literal_expr>(true);
    if (match(TT::T_NULL)) return std::make_unique<ast::literal_expr>(nullptr);

    if (match({TT::NUMBER, TT::STRING})) {
      return std::make_unique<ast::literal_expr>(previous().get_literal());
    }
    
    if (match(TT::IDENTIFIER)) {
      std::optional<ast::expr_ptr> object;
      auto name = previous();

      if(match(TT::LPAREN))
        return finish_call(object, name);
      else 
        return std::make_unique<ast::var_expr>(std::move(object), name);
    }

    if (match(TT::LPAREN)) {
      auto expr = expression();
      consume(TT::RPAREN, "Expected ')' after expression.");
      return expr;
    }

    if (match(TT::KW_NEW)) {
      auto name = consume(TT::IDENTIFIER, "Expected type name.");

      consume(TT::LPAREN, "Expected '(' after type name.");

      std::vector<ast::expr_ptr> args;
      if (!check(TT::RPAREN)) {
        do {
          auto expr = expression();
          args.push_back(std::move(expr));
        } while (match(TT::COMMA));
      }
      
      consume(TT::RPAREN, "Expected ')' after superclass arguments.");

      return std::make_unique<ast::new_expr>(name, std::move(args));
    }

    throw error(peek(), "Expected expression.");
  }

  static parse_error error(const lexer::token &token,
                           const std::string &message) {
    internal::error(token, message);
    return parse_error(message);
  }

  void sync() {
    advance();

    while (!is_at_end()) {
      if (previous().get_type() == TT::SEMICOLON) return;

      std::vector<lexer::token_type> types = {TT::KW_TYPE,  TT::KW_FUNCTION,
                                              TT::KW_LET,   TT::KW_IF,
                                              TT::KW_WHILE, TT::KW_FOR};
      for (auto &type : types) {
        if (check(type)) {
          return;
        }
      }

      advance();
    }
  }

  lexer::token consume(const lexer::token_type type,
                       const std::string &message) {
    if (check(type)) return advance();
    throw error(peek(), message);
  }

  template <class T>
  bool match(const std::initializer_list<T> &types) {
    for (auto &type : types) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }

  bool match(const lexer::token_type type) { return match({type}); }

  bool check(const lexer::token_type type) {
    return !is_at_end() && peek().get_type() == type;
  }

  lexer::token advance() {
    if (!is_at_end()) current++;
    return previous();
  }

  bool is_at_end() {
    return peek().get_type() == lexer::token_type::END_OF_FILE;
  }

  lexer::token peek() { return tokens.at(current); }
  lexer::token previous() { return tokens.at(current - 1); }
};

}  // namespace parser

}  // namespace hulk

#endif  // HULK_PARSER_H