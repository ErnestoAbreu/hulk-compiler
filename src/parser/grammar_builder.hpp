#ifndef HULK_GRAMMAR_BUILDER_HPP
#define HULK_GRAMMAR_BUILDER_HPP 1

#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

#include "../internal/internal_error"
#include "../lexer/tokens"
#include "gen_parser.hpp"

using namespace std;
using namespace hulk::lexer;

namespace hulk {

namespace gen_parser {

struct symbol {
  string repr;
  symbol(string s) : repr(s) {}
  symbol(const char *s) : repr(s) {}
  symbol(token_type tt) : repr(token_type_to_string(tt)) {}
};

struct grammar_builder {
  string start;
  unordered_map<string, vector<vector<string>>> grammar;

  grammar_builder(string _start = "") : start(_start) {}

  void set_start(string new_start) { start = new_start; }

  void add_production(string left, vector<symbol> right) {
    if (right.empty()) {
      add_epsilon_production(left);
      return;
    }
    grammar[left].emplace_back();
    for (auto &p : right)
      grammar[left].back().push_back(p.repr);
  }

  void add_epsilon_production(string left) {
    grammar[left].push_back({epsilon});
  }
};

struct hulk_grammar : public grammar_builder {
  hulk_grammar() : grammar_builder("program") { build_grammar(); }

 private:
  void build_grammar() {
    add_production("program", {"declaration_list"});

    add_production("declaration_list", {"declaration", "sep_semicolon", "declaration_list"});
    add_epsilon_production("declaration_list");

    add_production("sep_semicolon", {SEMICOLON});
    add_production("sep_semicolon", {D_SEMICOLON});

    add_production("declaration", {"function_decl"});
    add_production("declaration", {"type_decl"});
    add_production("declaration", {"protocol_decl"});
    add_production("declaration", {"stmt"});
    
    add_production("function_decl", {KW_FUNCTION, IDENTIFIER, LPAREN, "opt_param_list", RPAREN, "opt_type_annotation", "function_body"});
    
    add_production("opt_param_list", {"param_list"});
    add_epsilon_production("opt_param_list");
    
    add_production("param_list", {"param", "param_list_tail"});
    add_production("param_list_tail", {COMMA, "param", "param_list_tail"});
    add_production("param", {IDENTIFIER, "opt_type_annotation"});
    add_epsilon_production("param_list_tail");

    add_production("function_body", {ARROW, "expression"});
    add_production("function_body", {"block_expr"});

    add_production("type_decl", {KW_TYPE, IDENTIFIER, "type_params", "type_inheritance", LBRACE, "type_body", RBRACE});
    add_production("type_params", {LPAREN, "t_param_list", RPAREN});
    add_epsilon_production("type_params");

    add_production("t_param_list", {"t_param", "t_param_list_tail"});
    add_production("t_param_list_tail", {COMMA, "t_param", "t_param_list_tail"});
    add_production("t_param", {IDENTIFIER, "opt_type_annotation"});
    add_epsilon_production("t_param_list_tail");
    
    add_production("type_inheritance", {KW_INHERITS, IDENTIFIER, "parent_args"});
    add_epsilon_production("type_inheritance");
    add_production("parent_args", {LPAREN, "args", RPAREN});
    add_epsilon_production("parent_args");

    add_production("type_body", {"member", "type_body"});
    add_epsilon_production("type_body");

    add_production("member", {IDENTIFIER, "member_tail"});
    add_production("member_tail", {"opt_type_annotation", "opt_initializer", SEMICOLON});
    add_production("opt_initializer", {"initializer"});
    add_epsilon_production("opt_initializer");

    add_production("member_tail", {LPAREN, "opt_param_list", RPAREN, "opt_type_annotation", "function_body", "sep_semicolon"});

    add_production("protocol_decl", {KW_PROTOCOL, IDENTIFIER, "protocol_inheritance", LBRACE, "protocol_body", RBRACE});

    add_production("protocol_inheritance", {KW_EXTENDS, IDENTIFIER});
    add_epsilon_production("protocol_inheritance");
    add_production("protocol_body", {"protocol_method", "protocol_body_tail"});
    add_production("protocol_body_tail", {SEMICOLON, "protocol_method", "protocol_body_tail"});
    add_production("protocol_method", {IDENTIFIER, LPAREN, "method_params", RPAREN, "type_annotation", SEMICOLON});
    add_production("type_annotation", {COLON, IDENTIFIER});
    add_epsilon_production("protocol_body_tail");
    add_epsilon_production("protocol_body");
    
    add_production("method_params", {"m_param_list"});
    add_epsilon_production("method_params");

    add_production("m_param_list", {"m_param", "m_param_list_tail"});
    add_production("m_param_list_tail", {COMMA, "m_param", "m_param_list_tail"});
    add_production("m_param", {IDENTIFIER, "type_annotation"});
    add_epsilon_production("m_param_list_tail");

    add_production("stmt", {"expression_statement"});
    add_production("expression_statement", {"expression"});
    
    add_production("expression", {"let_expr"});
    add_production("expression", {"if_expr"});
    add_production("expression", {"while_expr"});
    add_production("expression", {"block_expr"});
    add_production("expression", {"for_expr"});
    add_production("expression", {"assignment"});

    add_production("opt_expression", {"expression"});
    add_epsilon_production("opt_expression");

    add_production("expression_list", {"opt_expression", "expression_list_tail"});
    add_production("expression_list_tail", {SEMICOLON, "opt_expression", "expression_list_tail"});
    add_epsilon_production("expression_list_tail");
    
    add_production("block_expr", {LBRACE, "expression_list", RBRACE});

    add_production("let_expr", {KW_LET, "binding_list", KW_IN, "expression"});

    add_production("binding", {IDENTIFIER, "opt_type_annotation", "initializer"});

    add_production("binding_list", {"binding", "binding_list_tail"});
    add_production("binding_list_tail", {COMMA, "binding", "binding_list_tail"});
    add_epsilon_production("binding_list_tail");

    add_production("opt_type_annotation", {COLON, IDENTIFIER});
    add_epsilon_production("opt_type_annotation");
  
    add_production("initializer", {OP_ASSIGN, "expression"});

    add_production("if_expr", {KW_IF, LPAREN, "expression", RPAREN, "expression", "elif_list", KW_ELSE, "expression"});
    add_production("elif_list", {KW_ELIF, LPAREN, "expression", RPAREN, "expression", "elif_list"});
    add_epsilon_production("elif_list");
    
    add_production("while_expr", {KW_WHILE, LPAREN, "expression", RPAREN, "expression"});
    add_production("for_expr", {KW_FOR, LPAREN, IDENTIFIER, "opt_type_annotation", KW_IN, "expression", RPAREN, "expression"});

    add_production("assignment", {"or_expr", "assignment_tail"});
    add_production("assignment_tail", {OP_DESTRUCT_ASSIGN, "or_expr", "assignment_tail"});
    add_epsilon_production("assignment_tail");

    add_production("or_expr", {"and_expr", "or_expr_tail"});
    add_production("or_expr_tail", {OP_OR, "and_expr", "or_expr_tail"});    
    add_epsilon_production("or_expr_tail");

    add_production("and_expr", {"equality", "and_expr_tail"});
    add_production("and_expr_tail", {OP_AND, "equality", "and_expr_tail"});
    add_epsilon_production("and_expr_tail");

    add_production("equality", {"comparison", "equality_tail"});
    add_production("equality_tail", {OP_NOT_EQUAL, "comparison", "equality_tail"});
    add_production("equality_tail", {OP_EQUAL, "comparison", "equality_tail"});
    add_epsilon_production("equality_tail");

    add_production("comparison", {"term", "comparison_tail"});
    add_production("comparison_tail", {OP_GREATER, "term", "comparison_tail"});
    add_production("comparison_tail", {OP_GREATER_EQ, "term", "comparison_tail"});
    add_production("comparison_tail", {OP_LESS, "term", "comparison_tail"});
    add_production("comparison_tail", {OP_LESS_EQ, "term", "comparison_tail"});
    add_epsilon_production("comparison_tail");

    add_production("term", {"factor", "term_tail"});
    add_production("term_tail", {OP_MINUS, "factor", "term_tail"});
    add_production("term_tail", {OP_PLUS, "factor", "term_tail"});
    add_epsilon_production("term_tail");

    add_production("factor", {"unary", "factor_tail"});
    add_production("factor_tail", {OP_DIVIDE, "unary", "factor_tail"});
    add_production("factor_tail", {OP_MODULE, "unary", "factor_tail"});
    add_production("factor_tail", {OP_MULTIPLY, "unary", "factor_tail"});
    add_production("factor_tail", {OP_EXPONENT, "unary", "factor_tail"});
    add_production("factor_tail", {OP_CONCAT, "unary", "factor_tail"});
    add_production("factor_tail", {OP_DOBLE_CONCAT, "unary", "factor_tail"});
    add_epsilon_production("factor_tail");

    add_production("unary", {OP_NOT, "unary"});
    add_production("unary", {OP_MINUS, "unary"});
    add_production("unary", {"var_call"});

    add_production("var_call", {"primary", "var_call_tail"});
    add_production("var_call_tail", {DOT, IDENTIFIER, "finish_call", "var_call_tail"});
    add_epsilon_production("var_call_tail");

    add_production("finish_call", {LPAREN, "args", RPAREN});
    add_epsilon_production("finish_call");

    add_production("args", {"expr_list"});
    add_epsilon_production("expr_list");
    add_production("expr_list", {"expression", "expr_list_tail"});
    add_production("expr_list_tail", {COMMA, "expression", "expr_list_tail"});
    add_epsilon_production("expr_list_tail");

    add_production("primary", {FALSE});
    add_production("primary", {TRUE});
    add_production("primary", {T_NULL});
    add_production("primary", {NUMBER});
    add_production("primary", {STRING});
    add_production("primary", {IDENTIFIER, "finish_call"});
    add_production("primary", {LPAREN, "expression", RPAREN});
    add_production("primary", {KW_NEW, IDENTIFIER, LPAREN, "args", RPAREN});
  }
};

};  // namespace gen_parser

}  // namespace hulk

#endif  // HULK_GRAMMAR_BUILDER_HPP
