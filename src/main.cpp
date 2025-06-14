#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "all"

using namespace hulk;

std::string read_source_code(const std::string& source_path) {
  const std::ifstream input(source_path, std::ios_base::binary);

  if (input.fail()) {
    throw std::runtime_error("Failed to open file");
  }

  std::stringstream buffer;
  buffer << input.rdbuf();

  return buffer.str();
}

int main(const int argc, char **argv) {
  auto source_code = read_source_code(argv[1]);

  if (source_code.empty()) {
    std::cout << "Source code must not be empty" << std::endl;
    return 255;
  }

  std::cerr << "START COMPILATION" << "\n";

  const auto& tokens = lexer::lex(source_code);

  std::cerr << "LEXED" << "\n";

  // for (auto &t: tokens) {
  //   std::cerr << t.to_string() << "\n";
  // }

  auto parser = parser::parser(tokens);
  const auto& ast = parser.parse();
  
  if (internal::error_found) return -1;

  std::cerr << "PARSED" << "\n";

  // std::cerr << ast.statements.size() << " declaration(s) found" << "\n";

  // if (ast.main.get()) {
  //   std::cerr << "Expression found" << "\n";
  // }

  if(semantic::analyze(ast))
    return -1;

  std::cerr << "ANALIZED" << "\n";

  return 0;
}