#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "regex_engine.cpp"
#include "tokens.cpp"

using namespace std;

vector<Token> lex(const string &input) {
  vector<pair<NFA, TokenType>> nfas;
  vector<Token> tokens;
  size_t pos = 0, n = input.size();
  int line = 1, column = 1;

  // Crear los NFA para cada regex
  for (auto &rt : RegexsTokens) {  // rt: <regex, type>
    Parser p(rt.first);
    nfas.push_back(make_pair(p.parse(), rt.second));
  }

  while (pos < n) {
    size_t max_len = 0;
    TokenType best_type = TokenType::UNKNOWN;

    for (size_t len = 1; pos + len <= n; ++len) {
      for (auto &nfa : nfas) {
        string sub = input.substr(pos, len);
        if (match(nfa.first, sub) && len > max_len) {
          max_len = len;
          best_type = nfa.second;
        }
      }
    }

    if (max_len == 0) {  // <=================== mejorar el manejo de errores
      // caracter inesperado
      exit(EXIT_FAILURE);
    }

    string lexeme = input.substr(pos, max_len);
    Token tok(best_type, lexeme, line, column);

    // actualizar línea/columna
    for (char c : lexeme) {
      if (c == '\n') {
        line++;
        column = 1;
      } else
        column++;
    }

    // agregar si no es WHITESPACE ni COMMENT
    if (best_type != TokenType::WHITESPACE && best_type != TokenType::COMMENT) {
      tokens.push_back(tok);
    }

    pos += max_len;
  }

  tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
  return tokens;
}
