#ifndef HULK_LEXER_HPP
#define HULK_LEXER_HPP 1

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>

#include "../lexer/tokens.hpp"
#include "regex_engine.hpp"

namespace hulk {

namespace lexer {

std::unordered_map<std::string, std::string> escape_mapping = {
    {"\\a", "\a"},  // Bell (alert)
    {"\\b", "\b"},  // Backspace
    {"\\t", "\t"},  // Horizontal tab
    {"\\n", "\n"},  // Newline
    {"\\v", "\v"},  // Vertical tab
    {"\\f", "\f"},  // Formfeed
    {"\\r", "\r"},  // Carriage return
    {"\\\"", "\""}, // Double quote
    {"\\'", "\'"},  // Single quote
    {"\\\\", "\\"}  // Backslash
};

std::vector<token> lex(const std::string &input) {
  std::vector<std::pair<nfa, token_type>> nfas;
  std::vector<token> tokens;
  size_t pos = 0, n = input.size();
  int line = 1, column = 1;

  // Crear los NFA para cada regex
  for (auto &rt : regexs_tokens) {  // rt: <regex, type>
    parser p(rt.first);
    nfas.push_back({p.parse(), rt.second});
  }

  while (pos < n) {
    size_t max_len = 0;
    token_type best_type = token_type::UNKNOWN;

    for (size_t len = 1; pos + len <= n; ++len) {
      for (auto &nfa : nfas) {
        std::string sub = input.substr(pos, len);
        if (match(nfa.first, sub) && len > max_len) {
          max_len = len;
          best_type = nfa.second;
        }
      }
    }

    if (max_len == 0) {  
      internal::lexical_error(line, column, "Unexpected caracter.");
      exit(0);
    }

    std::string lexeme = input.substr(pos, max_len);
    literal value;

    if (best_type == NUMBER) {  // number
      value = stod(lexeme);
    } else if (best_type == TRUE || best_type == FALSE) {  // boolean
      value = (best_type == TRUE ? true : false);
    } else {
      value = lexeme.substr(1, lexeme.size() - 2);
    }

    // actualizar línea/columna
    for (char c : lexeme) {
      if (c == '\n') {
        line++;
        column = 1;
      } else
        column++;
    }

    token tok(best_type, lexeme, value, line, column);

    // agregar si no es WHITESPACE ni COMMENT ni NEWLINE
    if (best_type != token_type::WHITESPACE &&
        best_type != token_type::COMMENT && best_type != token_type::NEWLINE) {
      tokens.push_back(tok);
    }

    pos += max_len;
  }

  tokens.push_back(token(token_type::END_OF_FILE, "", nullptr, line, column));
  return tokens;
}

}  // namespace lexer

}  // namespace hulk

#endif  // HULK_LEXER_HPP