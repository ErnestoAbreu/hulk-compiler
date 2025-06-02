#ifndef HULK_AUTOMATON_LR_HPP
#define HULK_AUTOMATON_LR_HPP 1

#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "../grammar/grammar.hpp"
#include "../internal/internal_str.hpp"
#include "item_lr1.hpp"
#include "table_lr1.hpp"

namespace hulk {

namespace automaton_lr1 {

typedef struct node_lr1 *pnode_lr1;
struct node_lr1 {
  int index;
  std::vector<pitem_lr1> items;
  std::map<grammar::grammar_token, int> transitions;

  node_lr1(int _index, std::vector<pitem_lr1> _items)
      : index(_index), items(_items) {}

  void add_transition(grammar::grammar_token token, pnode_lr1 v) {
    transitions[token] = v->index;
  }

  bool operator<(const node_lr1 &other) const { return index < other.index; }

  std::string to_string() {
    std::string result;
    for (auto item : items) result += item->to_string() + "\n";
    return result;
  }
};

struct automaton_lr1 {
  bool ok;
  std::string name;
  grammar::grammar grammar;
  std::vector<pitem_lr1> items;
  std::vector<pnode_lr1> nodes;

  automaton_lr1(std::string _name, grammar::grammar _grammar)
      : name(_name), grammar(_grammar) {
    build_grammar();
    build_items();
    build_nodes();

    ok = build_table(name);
  }

 private:
  void build_grammar() { grammar.calc_first(); }

  pitem_lr1 get_item(grammar::grammar_production production, int id,
                     grammar::grammar_token teal) {
    auto item = new item_lr1(items.size(), production, id, teal);
    items.push_back(item);
    return item;
  }

  pitem_lr1 get_item_main() {
    for (auto item : items) {
      if (item->production.head == grammar.main && item->id == 0 &&
          item->teal == grammar::eof)
        return item;
    }
    assert(false);  // todo: error (message: "item main not found")
  }

  void build_nodes() {
    auto item_main = get_item_main();
    std::vector<pitem_lr1> items_main = {item_main};
    build_closure(items_main);
    auto node = get_node(items_main);

    std::queue<pnode_lr1> q;
    q.push(node);
    while (!q.empty()) {
      auto x = q.front();
      q.pop();
      for (auto &t : grammar.get_tokens()) {
        if (t == grammar::eof) continue;

        auto go = build_goto(node->items, t);
        if (go.empty()) continue;

        auto [node_goto, to_add] = get_go_node(go);
        node->add_transition(t, node_goto);
        if (to_add) q.push(node_goto);
      }
    }
  }

  template <class T>
  bool in(std::vector<T> v, T x) {
    for (auto item : v) {
      if (v == x) return true;
    }
    return false;
  }

  void build_closure(std::vector<pitem_lr1> items) {
    std::queue<pitem_lr1> q;
    for (auto &item : items) q.push(item);
    while (!q.empty()) {
      auto item = q.front();
      q.pop();
      for (auto &i : item->get_eof_transitions()) {
        if (in(items, items[i])) continue;
        items.push_back(items[i]);
        q.push(items[i]);
      }
    }
  }

  std::vector<pitem_lr1> build_goto(std::vector<pitem_lr1> items,
                                    grammar::grammar_token token) {
    std::vector<pitem_lr1> go;
    for (auto &item : items) {
      for (auto &g : item->get_transitions(token)) {
        go.push_back(items[g]);
      }
    }
    build_closure(go);
    return go;
  }

  pnode_lr1 get_node(std::vector<pitem_lr1> closure) {
    auto node = new node_lr1(nodes.size(), closure);
    nodes.push_back(node);
    return node;
  }

  std::pair<pnode_lr1, bool> get_go_node(std::vector<pitem_lr1> items) {
    for (auto &node : nodes) {
      bool ok = true;
      for (auto &item : items) {
        if (in({begin(node->items), end(node->items)}, item)) continue;
        ok = false;
      }

      if (ok && size(node->items) == size(items)) return {node, false};
    }
    return {get_node(items), true};
  }

  void build_items() {
    std::map<grammar::grammar_token, std::vector<pitem_lr1>> head2item;
    std::map<std::pair<grammar::grammar_production, grammar::grammar_token>,
             std::vector<pitem_lr1>>
        tealprod2item;

    for (auto &prod : grammar.production) {
      for (int i = 0; i <= (int)prod.body.size(); i++) {
        for (auto &term : grammar.terminal) {
          auto item = get_item(prod, i, term);
          head2item[prod.head].push_back(item);
          tealprod2item[{prod, term}].push_back(item);
        }
      }
    }

    for (auto &item1 : items) {
      if (item1->production.body[item1->id].is_terminal ||
          item1->id == (int)item1->production.body.size())
        continue;

      for (auto &item2 : head2item[item1->production.body[item1->id]]) {
        if (item2->id == 0) {
          auto beta = get_suffix(item1->production.body, item1->id + 1);
          beta.push_back(item1->teal);

          auto w = grammar.calc_sentence_first(beta);
          if (w.count(item2->teal)) item1->add_eof_transition(item2);
        }
      }
    }

    for (auto &item1 : items) {
      if (item1->id == (int)item1->production.body.size()) continue;

      for (auto &item2 : tealprod2item[{item1->production, item1->teal}]) {
        if (item2->id == item1->id + 1) {
          item1->add_transition(item1->production.body[item1->id], item2);
        }
      }
    }
  }

  bool build_reduce(pnode_lr1 node, node_action node_action, bool result) {
    for (auto &item : node->items) {
      if (item->id == (int)item->production.body.size()) {
        if (item->production.head == grammar.main)
          result &= node_action.add_terminal_action(grammar::eof, "ACCEPT", -1);
        else
          result &= node_action.add_terminal_action(item->teal, "REDUCE",
                                                    item->production.index);
      }
      if (!result) break;
    }
    return result;
  }

  bool build_table(std::string name) {
    bool result = true;
    std::vector<node_action> node_actions;
    for (auto &node : nodes) {
      auto node_act = node_action(node->index);
      result = build_shift(node, node_act, result);
      result = build_reduce(node, node_act, result);
      if (!result) break;
      node_actions.push_back(node_act);
    }

    if (result)
      ;  // here export table_lr

    return result;
  }

  bool build_shift(pnode_lr1 node, node_action node_act, bool result) {
    for (auto &[t, i] : node->transitions) {
      if (t.is_terminal)
        result &= node_act.add_terminal_action(t, "SHIFT", i);
      else
        result &= node_act.add_nonterminal_action(t, i);

      if (!result) break;
    }
    return result;
  }

  template <class T>
  std::vector<T> get_suffix(const std::vector<T> &v, int i) {
    std::vector<T> result;
    for (; i < (int)v.size(); i++) result.push_back(v[i]);
    return result;
  }
};

}  // namespace automaton_lr1

}  // namespace hulk

#endif  // HULK_AUTOMATON_LR_HPP