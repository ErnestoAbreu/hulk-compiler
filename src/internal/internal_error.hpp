#ifndef HULK_INTERNAL_ERROR_HPP
#define HULK_INTERNAL_ERROR_HPP 1

#include <iostream>
#include <string>

#include "../lexer/tokens"

namespace hulk {
  namespace internal {

    static bool error_found = false;
    static unsigned int lib_lines = 0;

    static void print(const unsigned int line, const unsigned int column, const std::string where, const std::string message, std::string component = "") {
      std::cerr << "[" << line << ", " << column << "] error in" << component << " " << where << ": " << message << "\n";
      error_found = true;
    }

    static void error(const std::string message, const std::string component = "") {
      std::cerr << "Internal Error: " << component << " " << message << "\n";
      error_found = true;
    }

    static void error(const unsigned int line, const unsigned int column, const std::string message) {
      print(line, column, "", message);
    }

    static void error(const lexer::token& token, const std::string message) {
      if (token.get_type() == lexer::token_type::END_OF_FILE) {
        print(token.get_location().first, token.get_location().second, " at end", message);
      }
      else {
        print(token.get_location().first, token.get_location().second, " at '" + std::string(token.get_lexeme()) + "'", message);
      }
    }

    static void lexical_error(const unsigned int line, const unsigned int column, const std::string &message) {
      print(line, column, "", message, "lexer");
    }

  } // namespace internal

} // namespace hulk

#endif // HULK_INTERNAL_ERROR_HPP