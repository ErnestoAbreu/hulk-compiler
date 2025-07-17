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

std::string read_lib_code() {
  const std::ifstream input("lib.hulk", std::ios_base::binary);

  if (input.fail()) {
    throw std::runtime_error("Failed to open file");
  }

  std::stringstream buffer;
  buffer << input.rdbuf();

  return buffer.str();
}

int main(const int argc, char** argv) {
  auto lib_code = read_lib_code();
  auto source_code = read_source_code(argv[1]);

  // source_code = lib_code + source_code;

  if (source_code.empty()) {
    std::cout << "Source code must not be empty" << std::endl;
    return 255;
  }

  std::cerr << "START COMPILATION" << "\n";

  const auto& tokens = lexer::lex(source_code);

  if (internal::error_found) return -1;
   
  std::cerr << "LEXED" << "\n";

  gen_parser::hulk_grammar HG;

  auto parser = gen_parser::predictive_parser(HG.grammar, HG.start, false);

  auto success = parser.parse(tokens);

  if (internal::error_found) return -1;

  // parser.print_derivation_tree();

  gen_parser::ast_builder builder;
  const auto& ast = builder.build(parser.root.get());

  std::cerr << "PARSED" << "\n";
  
  if (semantic::analyze(ast))
    return -1;

  std::cerr << "ANALIZED" << "\n";

  code_generator::code_generator codegen;
  if (codegen.generate_code(ast))
    return -1;

  std::cerr << "CODE GENERATED" << "\n";

  std::cerr << "COMPILATION COMPLETED" << "\n";

  return 0;
}