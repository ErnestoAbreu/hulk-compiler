#ifndef HULK_ITEM_LR_HPP
#define HULK_ITEM_LR_HPP 1

#include <map>
#include <set>

#include "../grammar/grammar.hpp"

namespace hulk {

namespace automaton_lr1 {

typedef struct item_lr1* pitem_lr1;

struct item_lr1 {
  int node, id;
  grammar::grammar_production production;
  std::map<grammar::grammar_token, std::set<int>> transitions;
  grammar::grammar_token teal;
  
  item_lr1(int _node, grammar::grammar_production _production, int _id, grammar::grammar_token _teal)
      : node(_node), production(_production), id(_id), teal(_teal) {}

  void add_transition(grammar::grammar_token token, pitem_lr1 item) {
    transitions[token].insert(item->node);
  }

  void add_eof_transition(pitem_lr1 item) { add_transition(grammar::eof, item); }

  std::vector<int> get_transitions(grammar::grammar_token token) {
    std::vector<int> result;
    if (transitions.count(token)) {
      for (auto& v : transitions[token]) result.push_back(v);
    }
    return result;
  }

  std::vector<int> get_eof_transitions() {
    return get_transitions(grammar::eof);
  }

  std::string to_string() {
    return production.to_string() + " " + std::to_string(id) + " " + teal.to_string(); 
  }
};

}  // namespace automaton_lr1

}  // namespace hulk

#endif  // HULK_ITEM_LR_HPP