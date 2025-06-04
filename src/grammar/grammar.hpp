#ifndef HULK_GRAMMAR_HPP
#define HULK_GRAMMAR_HPP 1

#include <assert.h>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../internal/internal_str.hpp"

namespace hulk {

namespace grammar {

struct grammar_token {
  std::string value;
  bool is_terminal, is_main;

  grammar_token() {}
  grammar_token(std::string _value, bool _is_terminal = false,
                bool _is_main = false)
      : value(_value), is_terminal(_is_terminal), is_main(_is_main) {}

  bool operator==(const grammar_token &other) const {
    return value == other.value;
  }

  bool operator!=(const grammar_token &other) const {
    return !(value == other.value);
  }

  bool operator<(const grammar_token &other) const {
    return value < other.value;
  }

  std::string to_string() { return value; }
};

struct grammar_production {
  int index;
  grammar_token head;
  std::vector<grammar_token> body;

  grammar_production() {}
  grammar_production(int _index, grammar_token _head,
                     std::vector<grammar_token> _body)
      : index(_index), head(_head), body(_body) {}

  std::string to_string() {
    std::string result = head.to_string() + " -> ";
    for (auto &g : body) {
      result += g.to_string() + " ";
    }
    return result;
  }
};

const grammar_token eof = grammar_token("EOF", true);

struct grammar {
  grammar_token main;
  std::vector<grammar_production> production;
  std::set<grammar_token> terminal;
  std::set<grammar_token> nonterminal;
  std::map<grammar_token, std::set<grammar_token>> ffirst, ffollow;

  grammar() : terminal({eof}) {}

  grammar_production get_production(int index) const {
    return production[index];
  }

  std::vector<grammar_token> get_tokens() {
    std::vector<grammar_token> result;
    for (auto st : std::vector{nonterminal, terminal}) {
      for (const auto &t : st) {
        result.push_back(t);
      }
    }
    return result;
  }

  grammar_token get_token(std::string token_value) {
    for (auto st : std::vector{nonterminal, terminal}) {
      if (st.count(token_value)) {
        return *st.find(token_value);
      }
    }
    assert(false);  // todo: implement hulk_fail (message: "token_value not
                    // found in grammar")
  }

  void validate_nonterminal(std::string value) {
    if (islower(value[0]))
      assert(false);  // todo: implement hulk_fail (message: "non terminal must
                      // be in uppercase")
  }

  void add_main(std::string nonterm) {
    validate_nonterminal(nonterm);

    main = grammar_token(nonterm, false, true);
    nonterminal.insert(main);
  }

  void add_production(std::string nonterm, std::vector<std::string> sentences) {
    validate_nonterminal(nonterm);

    auto get = [&](std::string s) -> grammar_token {
      auto t = grammar_token(s, islower(s[0]));

      if (t.is_terminal)
        terminal.insert(t);
      else
        nonterminal.insert(t);

      return t == main ? main : t;
    };

    auto head = get_token(nonterm);

    for (auto &s : sentences) {
      auto tokens = internal::split(s);

      std::vector<grammar_token> body;
      for (auto &t : tokens)
        if (!t.empty() && t != "EOF") {
          body.push_back(get(t));
        }

      production.push_back(grammar_production(size(production), head, body));
    }
  }

  void calc_first() {
    ffirst.clear();
    for (auto &t : terminal) ffirst[t] = {t};

    while (true) {
      bool found = false;
      for (auto &p : production) {
        auto [_, head, body] = p;

        bool all_eps = true;
        for (auto &t : body) {
          for (auto &first : ffirst[t]) {
            if (!ffirst[head].count(first)) {
              ffirst[head].insert(first);
              found = true;
            }
          }

          if (!ffirst[t].count(eof)) {
            all_eps = false;
            break;
          }
        }

        if (all_eps && !ffirst[head].count(eof)) {
          ffirst[head].insert(eof);
          found = true;
        }
      }

      if (!found) break;
    }
  }

  std::set<grammar_token> calc_sentence_first(
      std::vector<grammar_token> tokens) {
    std::set<grammar_token> result;

    bool all_eps = true;
    for (auto &t : tokens) {
      for (auto &first : ffirst[t]) result.insert(first);

      if (!ffirst[t].count(eof)) {
        all_eps = false;
        break;
      }
    }

    if (all_eps && !result.count(eof)) result.insert(eof);

    return result;
  }

  void calc_follow() {
    ffollow.clear();
    calc_first();

    for (auto &t : nonterminal)
      if (t.is_main) {
        ffollow[t].insert(eof);
        break;
      }

    while (true) {
      bool found = false;
      for (auto &p : production) {
        auto [_, head, body] = p;

        std::deque<grammar_token> dbody = {begin(body), end(body)};
        for (auto &t : body) {
          if (!t.is_terminal) {
            dbody.pop_front();  // maintain suffix [i+1, size(body))

            auto sfirst = calc_sentence_first({begin(dbody), end(dbody)});
            for (auto f : sfirst)
              if (f != eof) {
                if (!ffollow[t].count(f)) {
                  ffollow[t].insert(f);
                  found = true;
                }
              }

            if (&t == &body.back() || sfirst.count(eof)) {
              for (auto &f : ffollow[head]) {
                if (!ffollow[t].count(f)) {
                  ffollow[t].insert(f);
                  found = true;
                }
              }
            }
          }
        }
      }

      if (!found) break;
    }
  }
};

}  // namespace grammar

}  // namespace hulk

#endif  // HULK_GRAMMAR_HPP