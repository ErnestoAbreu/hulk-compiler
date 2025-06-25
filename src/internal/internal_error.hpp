#ifndef HULK_INTERNAL_ERROR_HPP
#define HULK_INTERNAL_ERROR_HPP 1

#include <iostream>
#include <string>

#include "../lexer/tokens"

namespace hulk {

namespace internal {

  static bool error_found = false;
  static bool runtime_error_found = false;

  static void print(const unsigned int line, const unsigned int column, const std::string where, const std::string message) {
    std::cerr << "[" << line << ", " << column << "] Parse error" << where << ": " << message << "\n";
    error_found = true;
  }

  static void error(const unsigned int line, const unsigned int column, const std::string message) {
    print(line, column, "", message);
  }

  static void error(const lexer::token &token, const std::string message) {
    if (token.get_type() == lexer::token_type::END_OF_FILE) {
      print(token.get_location().first, token.get_location().second, " at end", message);
    } else {
      print(token.get_location().first, token.get_location().second, " at '" + std::string(token.get_lexeme()) + "'", message);
    }
  }
  
  struct runtime_error final : std::runtime_error {
    lexer::token token;
    explicit runtime_error(const lexer::token &_token, const std::string &_message) : std::runtime_error(_message), token(_token) {}
  };

  static void rte(const runtime_error &error) {
    std::cerr << error.what() << "\n[line " << error.token.get_location().first << "]";
    runtime_error_found = true;
  }

  static void semantic_error(const unsigned int line, unsigned int column, const std::string &message) {
    std::cerr << "[line " << line << ", column " << column << "] Semantic Error:\n\t" << message << "\n";
    error_found = true;
  }

} // namespace internal

} // namespace hulk

#endif // HULK_INTERNAL_ERROR_HPP