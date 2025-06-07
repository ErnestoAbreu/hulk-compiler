#ifndef HULK_AUTOMATON_HPP
#define HULK_AUTOMATON_HPP 1

#include <assert.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../internal/internal_str.hpp"

namespace hulk {

namespace automaton {

struct state {
  std::string name;
  bool terminal = false;
  std::unordered_map<std::string, std::vector<state*>> transitions;
  std::unordered_set<state*> epsilon_transitions;
  std::string tag;
  std::function<std::string(const std::string&)> formatter;

  state(
      const std::string& _name, bool _terminal = false,
      std::function<std::string(const std::string&)> _formatter =
          [](const std::string& x) { return x; })
      : name(_name), terminal(_terminal), formatter(_formatter) {}

  void set_formatter(const std::function<std::string(const std::string&)>& f,
                     std::unordered_set<state*>* vis = nullptr) {
    std::unordered_set<state*> _vis;
    if (!vis)
      vis = &_vis;
    else if (vis->find(this) != vis->end()) {
      return;
    }

    vis->insert(this);

    formatter = f;
    for (auto& s : transitions) {
      for (auto& state_ptr : s.second) {
        state_ptr->set_formatter(f, vis);
      }
    }

    for (auto& state_ptr : epsilon_transitions)
      state_ptr->set_formatter(f, vis);
  }

  bool has_transition(const std::string& symbol) const {
    return transitions.find(symbol) != end(transitions);
  }

  state& add_transition(const std::string& symbol, state*& _state) {
    transitions[symbol].push_back(_state);
    return *this;
  }

  state& add_epsilon_transition(state*& _state) {
    epsilon_transitions.insert(_state);
    return *this;
  }

  bool match(const std::vector<std::string>& str) {
    auto states = epsilon_closure();
    for (const auto& symbol : str) {
      states = move_by_state(symbol, states);
      states = epsilon_closure_by_state(states);
    }

    for (auto& s : states) {
      if (s->terminal) return true;
    }

    return false;
  }

  state* to_dfa(std::function<std::string(const std::string&)> formatter =
                    [](const std::string& x) { return x; }) {
    auto closure = epsilon_closure();

    bool has_terminal = false;
    std::vector<std::string> names;
    for (const auto& s : closure) {
      if (s->terminal) has_terminal = true;
      names.push_back(s->name);
    }

    std::sort(begin(names), end(names));

    std::string nstate_name = "";
    for (auto& name : names) {
      if (!nstate_name.empty()) nstate_name += ", ";
      nstate_name += name;
    }

    auto start = new state(nstate_name, has_terminal, formatter);

    std::vector<std::unordered_set<state*>> closures = {closure};
    std::vector<state*> dfa_states = {start};
    std::queue<state*> todo;
    todo.push(start);

    std::unordered_map<std::string, std::unordered_set<state*>> who;
    who[start->name] = closure;

    while (!todo.empty()) {
      auto cur = todo.front();
      todo.pop();

      std::unordered_set<std::string> symbols;
      for (const auto& s : who[cur->name]) {
        for (auto& [symbol, _] : s->transitions) {
          symbols.insert(symbol);
        }
      }

      for (auto& symbol : symbols) {
        auto move = move_by_state(symbol, who[cur->name]);
        auto nclosure = epsilon_closure_by_state(move);

        int index = -1;
        for (auto i = 0; i < closures.size(); i++) {
          if (closures[i] == nclosure) {
            index = i;
            break;
          }
        }

        if (index == -1) {
          bool nhas_terminal = false;
          std::vector<std::string> names;
          for (auto& s : nclosure) {
            if (s->terminal) nhas_terminal = true;
            names.push_back(s->name);
          }

          std::sort(begin(names), end(names));

          std::string nstate_name = "";
          for (auto& name : names) {
            if (!nstate_name.empty()) nstate_name += ", ";
            nstate_name += name;
          }

          auto nstate = new state(nstate_name, nhas_terminal, formatter);
          who[nstate_name] = nclosure;
          dfa_states.push_back(nstate);
          closures.push_back(nclosure);
          todo.push(nstate);
          cur->add_transition(symbol, nstate);
        } else {
          cur->add_transition(symbol, dfa_states[index]);
        }
      }
    }

    return start;
  }

  state* get(const std::string& symbol) {
    auto it = transitions.find(symbol);
    assert(it != transitions.end() && it->second.size() == 1);
    return it->second[0];
  }

  std::unordered_set<state*> operator[](const std::string& symbol) const {
    if (symbol.empty()) {
      return epsilon_transitions;
    }
    auto it = transitions.find(symbol);
    if (it != end(transitions)) {
      return std::unordered_set<state*>(it->second.begin(), it->second.end());
    }
    return {};
  }

  static std::unordered_set<state*> move_by_state(
      const std::string& symbol, const std::unordered_set<state*>& states) {
    std::unordered_set<state*> result;
    for (const auto& s : states) {
      if (s->has_transition(symbol)) {
        const auto& to = s->transitions.at(symbol);
        result.insert(to.begin(), to.end());
      }
    }
    return result;
  }

  std::unordered_set<state*> epsilon_closure() {
    return epsilon_closure_by_state({this});
  }

  static std::unordered_set<state*> epsilon_closure_by_state(
      const std::unordered_set<state*>& states) {
    std::unordered_set<state*> closure = states;
    size_t last;
    do {
      last = closure.size();
      auto temp = closure;
      for (const auto& s : temp) {
        closure.insert(s->epsilon_transitions.begin(),
                       s->epsilon_transitions.end());
      }
    } while (last != closure.size());
    return closure;
  }
};

std::string multiline_formatter(const std::string& name) {
  auto names = internal::split(name, ", ");
  std::string result = "";
  for (auto &s: names) {
    if (!result.empty()) result += "\n";
    result += s;
  }
  return result;
}

std::string lr0_formatter(const std::string& name) {
  auto names = internal::split(name, ", ");
  std::string result = "";
  for (auto &s: names) {
    if (!result.empty()) result += "\n";
    if (s.size() > 4)
      result += s.substr(0, s.size() - 4);
    else
      result += s;
  }
  return result;
}

}  // namespace automaton

}  // namespace hulk

#endif  // HULK_AUTOMATON_HPP