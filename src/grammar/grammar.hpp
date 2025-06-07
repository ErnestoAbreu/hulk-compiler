#ifndef HULK_GRAMMAR_HPP
#define HULK_GRAMMAR_HPP 1

#include <assert.h>

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

#include "../internal/internal_str.hpp"

namespace hulk {

namespace grammar {

struct grammar;
struct symbol;
struct terminal;
struct nonterminal;
struct eof;
struct sentence;
struct productions;
struct production;
struct item;

struct symbol {
  std::string name;
  grammar* grammar_ptr;

  symbol(const std::string& _name, grammar* _grammar)
      : name(_name), grammar_ptr(_grammar) {}

  virtual ~symbol() = default;

  virtual bool is_terminal() const { return false; }
  virtual bool is_nonterminal() const { return false; }
  virtual bool is_epsilon() const { return false; }

  virtual std::string to_string() const { return name; }
  virtual size_t hash() const { return std::hash<std::string>{}(name); }

  virtual sentence operator+(symbol* other);
  virtual std::vector<sentence> operator|(symbol* other);
};

struct terminal : public symbol {
  terminal(const std::string& name, grammar* grammar) : symbol(name, grammar) {}

  bool is_terminal() const override { return true; }
  bool is_nonterminal() const override { return false; }

  sentence operator+(symbol* other) override;
  std::vector<sentence> operator|(symbol* other) override;
};

struct nonterminal : public symbol {
  std::vector<struct production*> productions;

  nonterminal(const std::string& name, grammar* grammar)
      : symbol(name, grammar) {}

  bool is_terminal() const override { return false; }
  bool is_nonterminal() const override { return true; }

  nonterminal& operator%=(symbol* other);
  nonterminal& operator%=(const std::vector<sentence>& other);

  sentence operator+(symbol* other) override;
  std::vector<sentence> operator|(symbol* other) override;
};

struct eof : public terminal {
  eof(grammar* grammar) : terminal("$", grammar) {}
};

struct sentence {
  std::vector<symbol*> symbols;
  size_t hash_value;

  sentence(std::vector<symbol*> _symbols) {
    for (auto& s : _symbols)
      if (!s->is_epsilon()) symbols.push_back(s);
    hash_value = std::hash<std::string>{}(to_string());
  }

  std::string to_string() const {
    std::string result;
    for (auto& s : symbols) {
      result += s->to_string();
      if (&s != &symbols.back()) result += " ";
    }
    return result;
  }

  bool is_epsilon() const { return symbols.empty(); }
  size_t size() const { return symbols.size(); }
  size_t hash() const { return hash_value; }

  sentence operator+(symbol* other);
  sentence operator+(const sentence& other);
  std::vector<sentence> operator|(symbol* other);
  std::vector<sentence> operator|(const sentence& other);

  bool operator==(const sentence& other) const {
    if (symbols.size() != other.symbols.size()) return false;
    for (auto i = 0; i < symbols.size(); i++)
      if (symbols[i] != other.symbols[i]) return false;
    return true;
  }

  symbol* operator[](size_t index) const { return symbols[index]; }
};

struct epsilon : public terminal, public sentence {
  epsilon(grammar* grammar) : terminal("ε", grammar), sentence({}) {}

  std::string to_string() const override { return "ε"; }
  bool is_epsilon() const override { return true; }

  sentence operator+(symbol* other) override { return sentence({other}); }

  bool operator==(const epsilon& other) const { return true; }

  size_t hash() const { return std::hash<std::string>{}("ε"); }
};

struct production {
  nonterminal* head;
  sentence body;

  production(nonterminal* _head, const sentence& _body)
      : head(_head), body(_body) {}

  virtual std::string to_string() const {
    return head->to_string() + " → " + body.to_string();
  }

  bool is_epsilon() const { return body.is_epsilon(); }

  bool operator==(const production& other) const {
    return head == other.head && body == other.body;
  }

  size_t hash() const { return head->hash() ^ body.hash(); }
};

struct item {
  production* prod;
  size_t pos;
  symbol* lookahead;

  item() : prod(nullptr), pos(0), lookahead(nullptr) {}

  item(production* _prod, size_t _pos, symbol* _lookahead = nullptr)
      : prod(_prod), pos(_pos), lookahead(_lookahead) {}

  // Constructor de copia
  item(const item& other) = default;

  // Operador de asignación
  item& operator=(const item& other) = default;

  std::string to_string() const {
    std::string result;
    result += prod->head->to_string() + " → ";
    const auto& body = prod->body;
    for (auto i = 0; i < body.size(); i++) {
      if (i == pos) result += "•";
      result += body[i]->to_string();
      if (i != body.size() - 1) result += " ";
    }
    if (pos == body.size()) result += "•";

    if (lookahead) {
      result += ", " + lookahead->to_string();
    }

    return result;
  }

  bool operator==(const item& other) const {
    return prod == other.prod && pos == other.pos &&
           ((!lookahead && !other.lookahead) ||
            (lookahead && other.lookahead &&
             lookahead->name == other.lookahead->name));
  }

  size_t hash() const {
    size_t h = std::hash<production*>{}(prod) ^ pos;
    if (lookahead) h ^= std::hash<std::string>{}(lookahead->name);
    return h;
  }

  bool is_reduce_item() const { return pos == prod->body.size(); }

  symbol* next_symbol() const {
    return pos < prod->body.size() ? prod->body[pos] : nullptr;
  }

  item next_item() const { return item(prod, pos + 1, lookahead); }

  item core() const { return item(prod, pos); }
};

struct grammar {
  std::vector<production*> productions;
  std::vector<nonterminal*> nonterminals;
  std::vector<terminal*> terminals;
  nonterminal* start = nullptr;
  epsilon* geps;
  eof* geof;
  std::unordered_map<std::string, symbol*> symbols_map;

  grammar() {
    geps = new epsilon(this);
    geof = new eof(this);
    symbols_map["$"] = geof;
    symbols_map["ε"] = geps;
  }

  ~grammar() {
    delete geps;
    delete geof;
    for (auto p : productions) delete p;
    for (auto t : terminals) delete t;
    for (auto nt : nonterminals) delete nt;
  }

  nonterminal* get_nonterminal(const std::string& name, bool is_start = false) {
    std::string s = internal::strip(name);
    if (s.empty()) {
      assert(false);
    }

    if (symbols_map.find(s) != symbols_map.end()) {
      return dynamic_cast<nonterminal*>(symbols_map[s]);
    }

    auto term = new nonterminal(s, this);

    if (is_start) {
      if (this->start == nullptr)
        this->start = term;
      else {
        assert(false);
      }
    }

    nonterminals.push_back(term);
    symbols_map[s] = term;
    return term;
  }

  terminal* get_terminal(const std::string& name) {
    std::string s = internal::strip(name);
    if (s.empty()) {
      assert(false);
    }

    if (symbols_map.find(s) != symbols_map.end()) {
      return dynamic_cast<terminal*>(symbols_map[s]);
    }

    auto term = new terminal(s, this);

    terminals.push_back(term);
    symbols_map[s] = term;
    return term;
  }

  void add_production(production* production) {
    production->head->productions.push_back(production);
    productions.push_back(production);
  }

  symbol* operator[](const std::string& name) {
    auto it = symbols_map.find(name);
    return it != symbols_map.end() ? it->second : nullptr;
  }

  std::string to_string() const {
    std::string result;

    result += "Nonterminals:\n\t";
    for (auto& nt : nonterminals) {
      result += nt->to_string();
      if (&nt != &nonterminals.back()) result += ", ";
    }
    result += "\n";

    result += "Terminals:\n\t";
    for (auto& t : terminals) {
      result += t->to_string();
      if (&t != &terminals.back()) result += ", ";
    }
    result += "\n";

    result += "Productions:\n";
    for (auto& p : productions) {
      result += "\t" + p->to_string() + "\n";
    }

    return result;
  }

  grammar* augmented_grammar() {
    auto G = new grammar();
    std::unordered_map<symbol*, symbol*> symbol_map = {{geps, G->geps},
                                                       {geof, G->geof}};

    for (auto& t : terminals) {
      symbol_map[t] = G->get_terminal(t->name);
    }

    for (auto& nt : nonterminals) {
      symbol_map[nt] = G->get_nonterminal(nt->name);
    }

    auto new_start = G->get_nonterminal("S'", true);

    for (auto p : productions) {
      std::vector<symbol*> new_body;
      for (auto s : p->body.symbols) {
        new_body.push_back(symbol_map[s]);
      }
      G->add_production(new production(
          dynamic_cast<nonterminal*>(symbol_map[p->head]), sentence(new_body)));
    }

    G->add_production(
        new production(new_start, sentence({symbol_map[this->start]})));

    return G;
  }

  std::unordered_map<nonterminal*, std::unordered_set<terminal*>>
  compute_first_sets() {
    std::unordered_map<nonterminal*, std::unordered_set<terminal*>> first_sets;

    for (auto nt : nonterminals) {
      first_sets[nt] = {};
    }

    while (true) {
      bool change = false;
      for (auto p : productions) {
        auto head = p->head;
        auto& body = p->body;

        if (body.is_epsilon()) continue;

        size_t old_size = first_sets[head].size();

        bool all_have_epsilon = true;
        for (auto s : body.symbols) {
          if (s->is_terminal()) {
            first_sets[head].insert(dynamic_cast<terminal*>(s));
            all_have_epsilon = false;
            break;
          } else {
            auto nt = dynamic_cast<nonterminal*>(s);
            first_sets[head].insert(first_sets[nt].begin(),
                                    first_sets[nt].end());
            if (first_sets[nt].find(geps) == first_sets[nt].end()) {
              all_have_epsilon = false;
              break;
            }
          }
        }

        if (all_have_epsilon) {
          first_sets[head].insert(geps);
        }

        if (first_sets[head].size() != old_size) {
          change = true;
        }
      }
      if (!change) break;
    }

    return first_sets;
  }

  std::unordered_map<nonterminal*, std::unordered_set<terminal*>>
  compute_follow_sets() {
    auto first_sets = compute_first_sets();
    std::unordered_map<nonterminal*, std::unordered_set<terminal*>> follow_sets;

    for (auto nt : nonterminals) {
      follow_sets[nt] = {};
    }
    follow_sets[start].insert(geof);

    while (true) {
      bool change = false;
      for (auto p : productions) {
        auto head = p->head;
        auto& body = p->body;

        for (size_t i = 0; i < body.size(); i++) {
          if (!body[i]->is_nonterminal()) continue;
          auto B = dynamic_cast<nonterminal*>(body[i]);

          size_t old_size = follow_sets[B].size();

          if (i + 1 < body.size()) {
            auto beta = std::vector<symbol*>(body.symbols.begin() + i + 1,
                                             body.symbols.end());
            auto first_beta = compute_local_first(beta, first_sets);

            follow_sets[B].insert(first_beta.begin(), first_beta.end());
            follow_sets[B].erase(geps);

            if (first_beta.find(geps) != first_beta.end()) {
              follow_sets[B].insert(follow_sets[head].begin(),
                                    follow_sets[head].end());
            }
          } else {
            follow_sets[B].insert(follow_sets[head].begin(),
                                  follow_sets[head].end());
          }

          if (follow_sets[B].size() != old_size) {
            change = true;
          }
        }
      }
      if (!change) break;
    }

    return follow_sets;
  }

 private:
  std::unordered_set<terminal*> compute_local_first(
      const std::vector<symbol*>& seq,
      const std::unordered_map<nonterminal*, std::unordered_set<terminal*>>&
          first_sets) {
    std::unordered_set<terminal*> result;
    bool all_have_epsilon = true;

    for (auto s : seq) {
      if (s->is_terminal()) {
        result.insert(dynamic_cast<terminal*>(s));
        all_have_epsilon = false;
        break;
      } else {
        auto nt = dynamic_cast<nonterminal*>(s);
        result.insert(first_sets.at(nt).begin(), first_sets.at(nt).end());
        if (first_sets.at(nt).find(geps) == first_sets.at(nt).end()) {
          all_have_epsilon = false;
          break;
        }
      }
    }

    if (all_have_epsilon) {
      result.insert(geps);
    }

    return result;
  }
};

sentence symbol::operator+(symbol* other) { return sentence({this, other}); }

std::vector<sentence> symbol::operator|(symbol* other) {
  return std::vector<sentence>{sentence({this}), sentence({other})};
}

sentence terminal::operator+(symbol* other) { return sentence({this, other}); }

std::vector<sentence> terminal::operator|(symbol* other) {
  return std::vector<sentence>{sentence({this}), sentence({other})};
}

sentence nonterminal::operator+(symbol* other) {
  return sentence({this, other});
}

std::vector<sentence> nonterminal::operator|(symbol* other) {
  return std::vector<sentence>{sentence({this}), sentence({other})};
}

nonterminal& nonterminal::operator%=(symbol* other) {
  auto prod = new production(this, sentence({other}));
  grammar_ptr->add_production(prod);
  return *this;
}

nonterminal& nonterminal::operator%=(const std::vector<sentence>& other) {
  for (auto& s : other) {
    auto prod = new production(this, s);
    grammar_ptr->add_production(prod);
  }
  return *this;
}

sentence sentence::operator+(const sentence& other) {
  std::vector<symbol*> nsymbols = symbols;
  nsymbols.insert(nsymbols.end(), other.symbols.begin(), other.symbols.end());
  return sentence(nsymbols);
}

std::vector<sentence> sentence::operator|(symbol* other) {
  return std::vector<sentence>{*this, sentence({other})};
}

std::vector<sentence> sentence::operator|(const sentence& other) {
  return std::vector<sentence>{*this, other};
}

}  // namespace grammar

}  // namespace hulk

#endif  // HULK_GRAMMAR_HPP