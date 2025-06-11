#ifndef HULK_PARSER_H
#define HULK_PARSER_H 1

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "../ast/ast"
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
    auto program = ast::program();
    try {
      while (!is_at_end()) {
        if (auto decl = declaration(); decl.has_value()) {
          program.push_back(std::move(decl.value()));
        }
      }
    } catch (const std::invalid_argument &e) {
      std::cout << "error: " << e.what() << std::endl;
    }
    return program;
  }

 private:
  std::vector<lexer::token> tokens;

  int current = 0;

  using TT = lexer::token_type;

  using parserFn = ast::expr (parser::*)();

  std::optional<ast::stmt> declaration() {
    try {
      if (match(TT::KW_TYPE)) return class_declaration();
      if (match(TT::KW_FUNCTION)) return function(ast::function_type::FUNCTION);

      return std::make_optional<ast::stmt>(statement());
    } catch (parse_error &) {
      sync();
      return std::make_optional<ast::stmt>();
    }
  }

  ast::class_stmt_ptr class_declaration() {
    auto name = consume(TT::IDENTIFIER, "Expected class name.");

    std::vector<ast::parameter> parameters;
    if (match(TT::LPAREN)) {
      consume(TT::LPAREN, "Expected '(' after class name.");

      while (!check(TT::RPAREN)) {
        lexer::token param_name =
            consume(TT::IDENTIFIER, "Expected parameter name.");

        lexer::token param_type(TT::UNKNOWN, "", "", param_name.line,
                                param_name.column);
        if (match(TT::COLON)) {
          bool found = false;
          for (auto t : std::vector{TT::T_STRING, TT::T_BOOLEAN, TT::T_OBJECT,
                                    TT::T_CHAR, TT::T_NULL, TT::T_NUMBER}) {
            if (check(t)) {
              param_type = consume(t, "");
              found = true;
              break;
            }
          }

          if (!found) {
            error(param_type,
                  "Expected a type after ':' in parameter definition.");
          }
        }

        parameters.push_back({param_name, param_type});
      }

      consume(TT::RPAREN, "Expected ')' after parameters");
    }

    std::optional<ast::super_item_ptr> super_class;
    if (match(TT::KW_INHERITS)) {
      consume(TT::IDENTIFIER, "Expected super_class name.");
      auto super_name = previous();

      std::vector<ast::expr> args;
      if (match(TT::RPAREN)) {
        do {
          auto expr = expression();
          args.push_back(std::move(expr));
        } while (match(TT::COMMA));

        consume(TT::RPAREN, "Expected ')' after superclass arguments.");
      }

      super_class =
          std::make_unique<ast::super_item>(super_name, std::move(args));
    }

    consume(TT::LBRACE, "Expected '{' before class body.");

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

    consume(TT::RBRACE, "Expected '}' after class body.");

    return std::make_shared<ast::class_stmt>(
        name, std::move(super_class), std::move(fields), std::move(methods));
  }

  ast::field_stmt_ptr field_declaration(const lexer::token name) {
    ast::expr init = match(TT::OP_ASSIGN)
                         ? expression()
                         : std::make_unique<ast::literal_expr>(nullptr);
    consume(TT::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_shared<ast::field_stmt>(name, std::move(init));
  }

  ast::while_expr_ptr while_expression() {
    consume(TT::LPAREN, "Expected '(' after while.");
    ast::expr condition = expression();
    consume(TT::RPAREN, "Expected ')' after condition.");
    ast::expr body = expression();
    return std::make_unique<ast::while_expr>(std::move(condition),
                                             std::move(body));
  }

  ast::stmt statement() { return expression_statement(); }

  ast::stmt for_statement() {  // todo
    return ast::stmt();
  }

  ast::if_expr_ptr if_expression() {
    consume(TT::LPAREN, "Expected '(' after 'if'.");
    auto condition = expression();
    consume(TT::RPAREN, "Expected ')' after if condition.");

    auto then_branch = expression();
    std::vector<std::pair<ast::expr, ast::expr>> elif_branchs;
    while (match(TT::KW_ELIF)) {
      consume(TT::LPAREN, "Expected '(' after 'elif'.");
      auto condition = expression();
      consume(TT::RPAREN, "Expected ')' after elif condition");

      ast::expr cur_then_branch = expression();
      elif_branchs.emplace_back(std::move(condition),
                                std::move(cur_then_branch));
    }

    std::optional<ast::expr> else_branch = {};
    if (match(TT::KW_ELSE)) {
      else_branch = expression();
    }

    return std::make_unique<ast::if_expr>(
        std::move(condition), std::move(then_branch), std::move(elif_branchs),
        std::move(else_branch));
  }

  ast::expression_stmt_ptr expression_statement() {
    ast::expr expr = expression();
    // consume(TT::SEMICOLON, "Expected ';' after expression");
    return std::make_shared<ast::expression_stmt>(std::move(expr));
  }

  ast::function_stmt_ptr function(const ast::function_type type) {
    const std::string kind =
        type == ast::function_type::FUNCTION ? "function" : "method";

    lexer::token name = previous();
    if (kind == "function")
      name = consume(TT::IDENTIFIER, "Expected " + kind + " name.");

    consume(TT::LPAREN, "Expected '(' after " + kind + " name.");

    std::vector<ast::parameter> parameters;
    if (!check(TT::RPAREN)) {
      do {
        lexer::token param_name =
            consume(TT::IDENTIFIER, "Expected parameter name.");

        lexer::token param_type(TT::UNKNOWN, "", "", param_name.line,
                                param_name.column);
        if (match(TT::COLON)) {
          bool found = false;
          for (auto t : std::vector{TT::T_STRING, TT::T_BOOLEAN, TT::T_OBJECT,
                                    TT::T_CHAR, TT::T_NULL, TT::T_NUMBER}) {
            if (check(t)) {
              param_type = consume(t, "");
              found = true;
              break;
            }
          }

          if (!found) {
            error(param_type,
                  "Expected a type after ':' in parameter definition.");
          }
        }

        parameters.push_back({param_name, param_type});
      } while (match(TT::COMMA));
    }

    // for (auto &p: parameters) {
    //   std::cerr << p.name.to_string() << " " << p.type.to_string() << "\n";
    // }

    consume(TT::RPAREN, "Expected ')' after parameters.");

    ast::expr body;
    bool is_inline_fun = false;

    if (match(TT::ARROW)) {  // parse an inline fun
      is_inline_fun = true;
    } else {  // should be a full-form fun
      consume(TT::LBRACE, "Expected '{' before full-form" + kind + " body.");
    }

    body = expression();

    if (is_inline_fun) {
      consume(TT::SEMICOLON, "Expected ';' after inline " + kind + " body.");
    } else {
      consume(TT::RBRACE, "Expected '}' after full-form " + kind + " body.");
    }
    

    return std::make_shared<ast::function_stmt>(name, type, parameters,
                                                std::move(body));
  }

  ast::block_expr_ptr block() {
    std::vector<ast::expr> expressions;
    while (!check(TT::RBRACE) && !is_at_end()) {
      auto expr = expression();
      expressions.push_back(std::move(expr));
    }
    consume(TT::RBRACE, "Expected '}' after block.");
    return std::make_unique<ast::block_expr>(std::move(expressions));
  }

  ast::expr parse_binary_expr(
      const std::initializer_list<lexer::token_type> &types, ast::expr expr,
      const parserFn &f) {
    while (match(types)) {
      auto token = previous();
      expr = std::make_unique<ast::binary_expr>(
          std::move(expr), token, static_cast<ast::binary_op>(token.get_type()),
          std::invoke(f, this));
    }
    return expr;
  }

  ast::expr expression() {
    if (match(TT::KW_WHILE)) while_expression();
    if (match(TT::KW_IF)) return if_expression();
    if (match(TT::LBRACE)) return block();
    return assignment();
  }

  ast::expr let() {
    if (match(TT::KW_LET)) {
      std::vector<ast::assign_expr_ptr> decl;
      do {
        const lexer::token name =
            consume(TT::IDENTIFIER, "Expected variable name.");
        
        ast::expr init = match(TT::OP_ASSIGN)
                             ? expression()
                             : std::make_unique<ast::literal_expr>(nullptr);

        decl.push_back(
            std::make_unique<ast::assign_expr>(name, std::move(init)));
      } while (match(TT::COMMA));

      consume(TT::KW_IN,
              "Expected 'in' after variable declarations in let-in.");

      ast::expr value = expression();
  
      return std::make_unique<ast::let_expr>(std::move(decl), std::move(value));
    }

    return assignment();
  }

  ast::expr assignment() {
    auto expr = or_();
    if (match({TT::OP_ASSIGN, TT::OP_DESTRUCT_ASSIGN})) {
      const auto equals = previous();
      auto value = assignment();

      if (std::holds_alternative<ast::var_expr_ptr>(expr)) {
        const auto name = std::get<ast::var_expr_ptr>(expr)->name;
        return std::make_unique<ast::assign_expr>(name, std::move(value));
      }

      if (std::holds_alternative<ast::get_expr_ptr>(expr)) {
        const auto &get_expr = std::get<ast::get_expr_ptr>(expr);
        return std::make_unique<ast::set_expr>(
            std::move(get_expr->object), get_expr->name, std::move(value));
      }

      error(equals, "Invalid assignment target.");
    }
    return expr;
  }

  ast::expr or_() {
    auto expr = and_();
    while (match(TT::OP_OR)) {
      auto right = and_();
      expr = std::make_unique<ast::logical_expr>(
          std::move(expr), ast::logical_op::OR, std::move(right));
    }
    return expr;
  }

  ast::expr and_() {
    auto expr = equality();
    while (match(TT::OP_AND)) {
      auto right = equality();
      expr = std::make_unique<ast::logical_expr>(
          std::move(expr), ast::logical_op::AND, std::move(right));
    }
    return expr;
  }

  ast::expr equality() {
    return parse_binary_expr({TT::OP_NOT_EQUAL, TT::OP_EQUAL}, comparison(),
                             &parser::comparison);
  }

  ast::expr comparison() {
    return parse_binary_expr(
        {TT::OP_GREATER, TT::OP_GREATER_EQ, TT::OP_LESS, TT::OP_LESS_EQ},
        term(), &parser::term);
  }

  ast::expr term() {
    return parse_binary_expr({TT::OP_MINUS, TT::OP_PLUS}, factor(),
                             &parser::factor);
  }

  ast::expr factor() {
    return parse_binary_expr(
        {TT::OP_DIVIDE, TT::OP_MULTIPLY, TT::OP_EXPONENT, TT::OP_CONCAT},
        unary(), &parser::unary);
  }

  ast::expr unary() {
    if (match({TT::OP_NOT, TT::OP_MINUS})) {
      const auto token = previous();
      ast::expr right = unary();
      return std::make_unique<ast::unary_expr>(
          token, static_cast<ast::unary_op>(token.get_type()),
          std::move(right));
    }
    return call();
  }

  ast::expr call() {
    ast::expr expr = primary();
    while (true) {
      if (match(TT::LPAREN)) {
        expr = finish_call(expr);
      } else if (match(TT::DOT)) {
        lexer::token name =
            consume(TT::IDENTIFIER, "Expected property name after '.'.");
        expr = std::make_unique<ast::get_expr>(std::move(expr), name);
      } else
        break;
    }
    return expr;
  }

  ast::expr finish_call(ast::expr &callee) {
    std::vector<ast::expr> arguments;

    if (!check(TT::RPAREN)) {
      do {
        arguments.push_back(expression());
      } while (match(TT::COMMA));
    }

    consume(TT::RPAREN, "Expected ')' after arguments.");

    return std::make_unique<ast::call_expr>(std::move(callee), previous(),
                                            std::move(arguments));
  }

  ast::expr primary() {
    if (match(TT::FALSE)) return std::make_unique<ast::literal_expr>(false);
    if (match(TT::TRUE)) return std::make_unique<ast::literal_expr>(true);
    if (match(TT::T_NULL)) return std::make_unique<ast::literal_expr>(nullptr);

    if (match({TT::NUMBER, TT::STRING})) {
      return std::make_unique<ast::literal_expr>(previous().get_literal());
    }

    if (match(TT::IDENTIFIER)) {
      return std::make_unique<ast::var_expr>(previous());
    }

    if (match(TT::LPAREN)) {
      auto expr = expression();
      consume(TT::RPAREN, "Expected ')' after expression.");
      return std::make_unique<ast::grouping_expr>(std::move(expr));
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