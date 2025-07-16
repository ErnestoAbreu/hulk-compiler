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
    grammar[left].emplace_back();
    for (auto &p : right)
      grammar[left].back().push_back(p.repr);
  }

  void add_epsilon_production(string left) {
    grammar[left].push_back({epsilon});
  }
};

struct hulk_grammar : public grammar_builder {
  hulk_grammar() : grammar_builder("program") { build_hulk_grammar(); }

 private:
  void build_hulk_grammar() {
    // Programa principal
    add_production("program", {"declaration_list"});
    add_production("declaration_list",
                   {"declaration", "declaration_list_tail"});
    add_production("declaration_list_tail",
                   {SEMICOLON, "declaration", "declaration_list_tail"});
    add_epsilon_production("declaration_list_tail");

    // Declaraciones
    add_production("declaration", {"function_decl"});
    add_production("declaration", {"type_decl"});
    add_production("declaration", {"protocol_decl"});
    add_production("declaration", {"stmt"});

    // Funciones
    add_production("function_decl",
                   {KW_FUNCTION, IDENTIFIER, LPAREN, "params", RPAREN,
                    "type_annotation", "function_body"});
    add_production("params", {"param_list"});
    add_epsilon_production("params");
    add_production("param_list", {"param", "param_list_tail"});
    add_production("param_list_tail", {COMMA, "param", "param_list_tail"});
    add_epsilon_production("param_list_tail");
    add_production("param", {IDENTIFIER, "type_annotation"});

    // Tipos
    add_production("type_decl", {KW_TYPE, IDENTIFIER, "inheritance", LBRACE,
                                 "type_body", RBRACE});
    add_production("inheritance", {KW_INHERITS, IDENTIFIER, "parent_args"});
    add_epsilon_production("inheritance");
    add_production("parent_args", {LPAREN, "args", RPAREN});
    add_epsilon_production("parent_args");
    add_production("type_body", {"member", "type_body_tail"});
    add_epsilon_production("type_body");
    add_production("type_body_tail", {SEMICOLON, "member", "type_body_tail"});
    add_epsilon_production("type_body_tail");
    add_production("member", {IDENTIFIER, "type_annotation", "initializer"});
    add_production("member", {"function_decl"});

    // Protocolos
    add_production("protocol_decl",
                   {KW_PROTOCOL, IDENTIFIER, "protocol_inheritance", LBRACE,
                    "protocol_body", RBRACE});
    add_production("protocol_inheritance", {KW_EXTENDS, IDENTIFIER});
    add_epsilon_production("protocol_inheritance");
    add_production("protocol_body", {"protocol_method", "protocol_body_tail"});
    add_epsilon_production("protocol_body");
    add_production("protocol_body_tail",
                   {SEMICOLON, "protocol_method", "protocol_body_tail"});
    add_epsilon_production("protocol_body_tail");
    add_production("protocol_method",
                   {IDENTIFIER, LPAREN, "params", RPAREN, "type_annotation"});

    // Anotaciones de tipo
    add_production("type_annotation", {COLON, IDENTIFIER});
    add_epsilon_production("type_annotation");

    // Cuerpo de función
    add_production("function_body", {ARROW, "expr"});
    add_production("function_body", {LBRACE, "stmt_list", RBRACE});

    // Inicializadores
    add_production("initializer", {OP_ASSIGN, "expr"});
    add_epsilon_production("initializer");

    // Declaraciones
    add_production("stmt", {"let_stmt"});
    add_production("stmt", {"if_stmt"});
    add_production("stmt", {"while_stmt"});
    add_production("stmt", {"for_stmt"});
    add_production("stmt", {"block"});
    add_production("stmt", {"expr_stmt"});

    // Let
    add_production("let_stmt", {KW_LET, "binding_list", KW_IN, "expr"});
    add_production("binding_list", {"binding", "binding_list_tail"});
    add_production("binding_list_tail",
                   {COMMA, "binding", "binding_list_tail"});
    add_epsilon_production("binding_list_tail");
    add_production("binding", {IDENTIFIER, "type_annotation", "initializer"});

    // If
    add_production("if_stmt", {KW_IF, LPAREN, "expr", RPAREN, "expr",
                               "elif_list", "else_clause"});
    add_production("elif_list",
                   {KW_ELIF, LPAREN, "expr", RPAREN, "expr", "elif_list"});
    add_epsilon_production("elif_list");
    add_production("else_clause", {KW_ELSE, "expr"});
    add_epsilon_production("else_clause");

    // While
    add_production("while_stmt", {KW_WHILE, LPAREN, "expr", RPAREN, "expr"});

    // For
    add_production("for_stmt",
                   {KW_FOR, LPAREN, "binding", KW_IN, "expr", RPAREN, "expr"});

    // Bloques
    add_production("block", {LBRACE, "stmt_list", RBRACE});
    add_production("stmt_list", {"stmt", "stmt_list_tail"});
    add_production("stmt_list_tail", {SEMICOLON, "stmt", "stmt_list_tail"});
    add_epsilon_production("stmt_list_tail");

    // Expresiones
    add_production("expr_stmt", {"expr"});
    add_production("expr", {"assignment"});

    // Asignaciones
    add_production("assignment", {"logic_or", "assignment_tail"});
    add_production("assignment_tail", {OP_DESTRUCT_ASSIGN, "assignment"});
    add_epsilon_production("assignment_tail");

    // Lógica
    add_production("logic_or", {"logic_and", "logic_or_tail"});
    add_production("logic_or_tail", {OP_OR, "logic_and", "logic_or_tail"});
    add_epsilon_production("logic_or_tail");
    add_production("logic_and", {"equality", "logic_and_tail"});
    add_production("logic_and_tail", {OP_AND, "equality", "logic_and_tail"});
    add_epsilon_production("logic_and_tail");

    // Igualdad
    add_production("equality", {"comparison", "equality_tail"});
    add_production("equality_tail",
                   {"equality_op", "comparison", "equality_tail"});
    add_epsilon_production("equality_tail");
    add_production("equality_op", {OP_EQUAL});
    add_production("equality_op", {OP_NOT_EQUAL});

    // Comparación
    add_production("comparison", {"term", "comparison_tail"});
    add_production("comparison_tail",
                   {"comparison_op", "term", "comparison_tail"});
    add_epsilon_production("comparison_tail");
    add_production("comparison_op", {OP_LESS});
    add_production("comparison_op", {OP_LESS_EQ});
    add_production("comparison_op", {OP_GREATER});
    add_production("comparison_op", {OP_GREATER_EQ});

    // Términos
    add_production("term", {"factor", "term_tail"});
    add_production("term_tail", {"add_op", "factor", "term_tail"});
    add_epsilon_production("term_tail");
    add_production("add_op", {OP_PLUS});
    add_production("add_op", {OP_MINUS});

    // Factores
    add_production("factor", {"unary", "factor_tail"});
    add_production("factor_tail", {"mult_op", "unary", "factor_tail"});
    add_epsilon_production("factor_tail");
    add_production("mult_op", {OP_MULTIPLY});
    add_production("mult_op", {OP_DIVIDE});
    add_production("mult_op", {OP_MODULE});
    add_production("mult_op", {OP_EXPONENT});
    add_production("mult_op", {OP_CONCAT});
    add_production("mult_op", {OP_DOBLE_CONCAT});

    // Unarios
    add_production("unary", {"unary_op", "unary"});
    add_production("unary", {"call"});
    add_production("unary_op", {OP_NOT});
    add_production("unary_op", {OP_MINUS});

    // Llamadas
    add_production("call", {"primary", "call_tail"});
    add_production("call_tail", {DOT, IDENTIFIER, "call_suffix"});
    add_epsilon_production("call_tail");
    add_production("call_suffix", {LPAREN, "args", RPAREN});
    add_production("call_suffix", {});  // Acceso a propiedad

    // Argumentos
    add_production("args", {"expr_list"});
    add_epsilon_production("args");
    add_production("expr_list", {"expr", "expr_list_tail"});
    add_production("expr_list_tail", {COMMA, "expr", "expr_list_tail"});
    add_epsilon_production("expr_list_tail");

    // Primarios
    add_production("primary", {TRUE});
    add_production("primary", {FALSE});
    add_production("primary", {T_NULL});
    add_production("primary", {NUMBER});
    add_production("primary", {STRING});
    add_production("primary", {IDENTIFIER, "identifier_tail"});
    add_production("primary", {LPAREN, "expr", RPAREN});
    add_production("primary", {KW_NEW, IDENTIFIER, LPAREN, "args", RPAREN});
    add_production("identifier_tail", {LPAREN, "args", RPAREN});
    add_epsilon_production("identifier_tail");
  }
};

};  // namespace gen_parser

}  // namespace hulk

#endif  // HULK_GRAMMAR_BUILDER_HPP