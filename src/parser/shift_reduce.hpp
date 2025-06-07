#ifndef HULK_SHIFT_REDUCE_HPP
#define HULK_SHIFT_REDUCE_HPP 1

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

#include "../grammar/grammar.hpp"
#include "../lexer/tokens.hpp"

namespace hulk {

namespace parser {

struct shift_reduce_parser {
  static const std::string SHIFT;
  static const std::string REDUCE;
  static const std::string OK;

  grammar::grammar G;
  std::unordered_map<std::pair<int, std::string>, std::pair<std::string, int>>
      action_table;
  std::unordered_map<std::pair<int, std::string>, int> goto_table;

  shift_reduce_parser(grammar::grammar _G)
      : G(_G), action_table({}), goto_table({}) {
    build_parsing_table();
  }

  virtual void build_parsing_table() = 0;

  struct parse_result {
    std::vector<grammar::production*> output;
    std::vector<std::string> operations;
    bool success;
    std::string error_message;
  };

  parse_result operator()(const std::vector<Token>& tokens,
                          bool get_shift_reduce = false) {
    std::stack<int> st;
    st.push(0);

    size_t current = 0;
    std::vector<grammar::production*> output;
    std::vector<std::string> operations;

    while (current <= tokens.size()) {
      int state = st.top();
      std::string str = (current < tokens.size()) ? 
                        TokenType_to_string(tokens[current].type) : "$";

      auto action_key = std::make_pair(state, str);
      auto action_it = action_table.find(action_key);

      if (action_it != action_table.end()) {
        auto [action, tag] = action_it->second;
        
        if (get_shift_reduce) {
          operations.push_back(action);
        }

        if (action == OK) {
          return {output, operations, true, ""};
        }
        else if (action == SHIFT) {
          st.push(tag);
          current++;
        }
        else if (action == REDUCE) {
          auto prod = G.productions[tag];
          output.push_back(prod);

          // Pop the right-hand side symbols
          for (size_t i = 0; i < prod->body.size(); i++) {
            if (!prod->body[i]->is_epsilon()) {
              st.pop();
            }
          }

          // Get the new state after reduction
          int new_state = st.top();
          auto goto_key = std::make_pair(new_state, prod->head->name);
          auto goto_it = goto_table.find(goto_key);

          if (goto_it != goto_table.end()) {
            st.push(goto_it->second);
          } else {
            std::string message = "No GOTO entry for " + prod->head->name + 
                                " in state " + std::to_string(new_state);
            return {output, operations, false, message};
          }
        }
      } else {
        // Error handling - build expected tokens list
        std::vector<std::string> expected;
        for (const auto& [key, value] : action_table) {
          if (key.first == state) {
            expected.push_back(key.second);
          }
        }

        std::string next_tokens;
        for (auto i = current; i < tokens.size() && i < current + 3; i++) {
          next_tokens += tokens[i].value + " ";
        }

        std::string message = "Syntax error at line " + 
                            std::to_string(tokens[current].line) + 
                            ", column " + 
                            std::to_string(tokens[current].column) + 
                            "\nExpected one of: ";
        
        for (auto& e : expected) {
          message += e + " ";
        }
        
        message += "\nBefore: " + next_tokens;

        return {output, operations, false, message};
      }
    }

    return {output, operations, false, "Unexpected end of input"};
  }
};

const std::string shift_reduce_parser::SHIFT = "shift";
const std::string shift_reduce_parser::REDUCE = "reduce";
const std::string shift_reduce_parser::OK = "accept";

}  // namespace parser

}  // namespace hulk

#endif  // HULK_SHIFT_REDUCE_HPP