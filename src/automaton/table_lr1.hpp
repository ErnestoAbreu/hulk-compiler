#ifndef HULK_TABLE_LR_HPP
#define HULK_TABLE_LR_HPP 1

#include <map>
#include <set>
#include <vector>

#include "../grammar/grammar.hpp"

namespace hulk {

namespace automaton_lr1 {

struct node_action {
  int index;
  std::map<grammar::grammar_token, std::pair<std::string, int>>
      terminal_actions;
  std::map<grammar::grammar_token, int> nonterminal_actions;

  node_action(int _index) : index(_index) {}

  bool add_terminal_action(grammar::grammar_token token, std::string action,
                           int index) {
    if (terminal_actions.count(token)) return false;

    terminal_actions[token] = {action, index};
    return true;
  }

  bool add_nonterminal_action(grammar::grammar_token token, int index) {
    if (nonterminal_actions.count(token)) return false;

    nonterminal_actions[token] = index;
    return true;
  }
};


struct table_lr {
  grammar::grammar grammar;
  std::vector<int> stack_states;
  std::vector<node_action> node_actions;

  table_lr(grammar::grammar _grammar) : grammar(_grammar), stack_states({0}) {}

  void reset() { stack_states = {0}; }

  std::pair<std::string, int> action(grammar::grammar_token token) {
    auto node = node_actions[stack_states.back()];
    if (node.terminal_actions.count(token)) {
      auto [action, index] = node.terminal_actions[token];
      if (action == "SHIFT") return action_shift(action, index);
      if (action == "REDUCE") return action_reduce(action, index);
      if (action == "ACCEPT") return {action, -1};
    }
    return {"ERROR", -1};
  }

  std::pair<std::string, int> action_shift(std::string action, int index) {
    stack_states.push_back(index);
    return {action, -1};
  }

  std::pair<std::string, int> action_reduce(std::string action, int index) {
    auto production = grammar.get_production(index);
    for (auto &t : production.body) stack_states.pop_back();
    stack_states.push_back(
        node_actions[stack_states.back()].nonterminal_actions[production.head]);
  }
};

}  // namespace automaton_lr1

}  // namespace hulk

#endif  // HULK_TABLE_LR_HPP