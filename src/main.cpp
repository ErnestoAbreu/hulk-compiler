#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

int main() {
  auto source_code = read_source_code("prog.hulk");

  if (source_code.empty()) {
    std::cout << "Source code must not be empty" << std::endl;
    return 255;
  }

  const auto& tokens = lexer::lex(source_code);

  for (auto &t: tokens) {
    std::cout << t.to_string() << "\n";
  }
}