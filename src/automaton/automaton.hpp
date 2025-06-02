#ifndef HULK_AUTOMATON_HPP
#define HULK_AUTOMATON_HPP 1

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../internal/internal_str.hpp"

namespace hulk {

namespace automaton {

typedef struct state *pstate;
struct state {
  int index;
  bool is_final;
  std::map<std::string, pstate> transition;
  std::set<pstate> eof_transition;
  pstate complement;

  state(int _index, bool _is_final = false)
      : index(_index), is_final(_is_final) {
    transition.clear();
    eof_transition.clear();
    complement = nullptr;
  }

  void add_transition(std::string s, pstate state) { transition[s] = state; }

  void add_eof_transition(pstate state) { eof_transition.insert(state); }

  pstate go(std::string s) {
    return (transition.count(s)) ? transition[s] : complement;
  }

  std::vector<pstate> go_eof() {
    return {begin(eof_transition), end(eof_transition)};
  }

  bool operator<(const state &other) const { return index < other.index; }
};

struct automaton {
  bool copy;
  pstate initial_state;
  std::vector<pstate> states;

  automaton(bool _copy = false) : copy(_copy) {
    initial_state = copy ? nullptr : new state(0);
    if (!copy) states.push_back(initial_state);
  }

  void add_transition(pstate from, pstate to, std::string s) {
    from->add_transition(s, to);
  }

  void add_eof_transition(pstate from, pstate to) {
    from->add_eof_transition(to);
  }

  void add_final_state(pstate state) {
    state->is_final = true;
  }

  void add_complement(pstate from, pstate to) { from->complement = to; }

  pstate get_new_state(pstate s = nullptr) {
    auto result = s != nullptr ? s : new state(size(states));
    states.push_back(result);
    return result;
  }

  bool match(std::string str) {
    std::set<std::pair<int, size_t>> seen;
    auto dfs = [&](auto &&self, pstate state, size_t i) -> bool {
      if (i == size(str)) return state->is_final;

      if (seen.count({state->index, i})) return false;
      seen.insert({state->index, i});

      for (auto &eof_state : state->eof_transition) {
        if (self(self, eof_state, i)) return true;
      }

      auto nxt = state->go("" + str[i]);

      if (nxt) return self(self, nxt, i + 1);

      return false;
    };

    return dfs(dfs, initial_state, 0);
  }
};

automaton copy(automaton aut) {
  auto result = automaton(true);
  for (size_t i = 0; i < size(aut.states); i++) result.get_new_state();

  result.initial_state = result.states[aut.initial_state->index];

  for (auto &from : aut.states) {
    for (auto &eof_state : from->eof_transition) {
      result.add_eof_transition(result.states[from->index],
                                result.states[eof_state->index]);
    }

    for (auto &[s, to] : from->transition) {
      result.add_transition(result.states[from->index],
                            result.states[to->index], s);
    }

    result.states[from->index]->is_final = from->is_final;
    result.states[from->index]->complement =
        from->complement ? result.states[from->complement->index] : nullptr;
  }

  return result;
}

automaton join(automaton a, automaton b) {
  b = copy(b);
  a.add_eof_transition(a.initial_state, b.initial_state);
  for (auto &s : b.states) a.get_new_state(s);
  return a;
}

automaton concat(automaton a, automaton b) {
  b = copy(b);
  for (auto &s : a.states) if (s->is_final) {
    a.add_eof_transition(s, b.initial_state);
    s->is_final = false;
  }
  for (auto &s : b.states) a.get_new_state(s);
  return a;
}

automaton many(automaton a) {
  for (auto &s: a.states) if (s->is_final) {
    a.add_eof_transition(s, a.initial_state);
  }
  a.initial_state->is_final = true;
  return a;
}

automaton to_dfa(automaton a) {
  // todo
}

}  // namespace automaton

}  // namespace hulk

#endif  // HULK_AUTOMATON_HPP