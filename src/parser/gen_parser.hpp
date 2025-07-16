#ifndef HULK_GEN_PARSER_HPP
#define HULK_GEN_PARSER_HPP 1

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

#include "../lexer/tokens"
#include "../internal/internal_error"

using namespace std;
using namespace hulk::lexer;

namespace hulk {

namespace gen_parser {

struct parse_error final : public std::runtime_error {
  size_t line;
  size_t column;
  explicit parse_error(const std::string& message, size_t _line = 0, size_t _column = 0) 
    : runtime_error(message), line(_line), column(_column) {}
};

const string epsilon = "ε", eof_symb = "$";

struct predictive_parser {
  predictive_parser(const unordered_map<string, vector<vector<string>>>& grammar,
         const string& start, bool _verbose = false)
      : productions(grammar), start_symbol(start), verbose(_verbose) {
    compute_first();
    compute_follow();
    build_parsing_table();
    init_error_handlers();
  }

  void init_error_handlers() {
    for (const auto& row : parsing_table) {
      for (const auto& entry : row.second) {
        sync_symbols.insert(entry.first);
      }
    }
  }

  void build_parsing_table() {
    for (const auto& prod : productions) {
      const string& A = prod.first;
      for (const auto& alpha : prod.second) {
        unordered_set<string> firstAlpha = compute_firsts(alpha);

        // for each terminal a in FIRST(α) add A -> α to M[A, a]
        for (const auto& a : firstAlpha) {
          if (a != epsilon) {
            if (parsing_table[A].count(a) && parsing_table[A][a] != alpha) {
              string error_message =
                  "Grammar is not LL(1)! Conflict at M[" + A + ", " + a + "]";
              internal::error(error_message, "parser");
              return;
            }
            parsing_table[A][a] = alpha;
          }
        }

        // If ε is in FIRST(α), add A -> α to M[A, b] for each b in FOLLOW(A)
        if (firstAlpha.count(epsilon)) {
          for (const auto& b : follow[A]) {
            if (parsing_table[A].count(b) && parsing_table[A][b] != alpha) {
              string error_message =
                  "Grammar is not LL(1)! Conflict at M[" + A + ", " + b + "]";
              internal::error(error_message, "parser");
              return;
            }
            parsing_table[A][b] = alpha;
          }
        }
      }
    }
  }

  bool parse(const vector<token>& tokens) {
    vector<string> input;
    for (auto &x: tokens)
      input.push_back(x.type == END_OF_FILE ? "$" : token_type_to_string(x.type));

    bool success = true;

    vector<string> w = input;

    for (auto &v: w) cout << v << "\n";

    stack<string> st;
    st.push(eof_symb); st.push(start_symbol);

    size_t ip = 0; 
    string X = st.top();

    if (verbose) {
      cout << "Parsing steps:\n";
      cout << "Stack\t\tInput\t\tAction\n";
    }

    while (X != eof_symb) {
      string a = w[ip];

      if (verbose) {
        cout << stack_to_string(st) << "\t" << input_to_string(w, ip) << "\n";
      }

      if (X == a) {
        st.pop();
        ip++;

        if (verbose) cout << "Match " << X << endl;
      } else if (!is_nonterminal(X)) {
        error(X, "Unexpected token '" + X + "'");
        success = false;
        
        if (!handle_unexpected_terminal(st, X))
          return success;

      } else if (!parsing_table[X].count(a)) {
        error(X, "Unexpected token '" + a + "' on " + X);
        success = false;

        if (!handle_missing_production(st, w, ip, X, a))
          return success;

      } else {
        vector<string> production = parsing_table[X][a];
        st.pop();

        if (production[0] != epsilon) {
          for (auto it = production.rbegin(); it != production.rend(); it++) {
            st.push(*it);
          }
        }

        if (verbose) {
          cout << "Apply " << X << " -> ";
          for (const string& symb: production) {
            cout << symb << " ";
          }
          cout << endl;
        } 
      }
      
      X = st.top();
    }
    
    success |= w[ip] == eof_symb;

    return success;
  }

  void print_parsing_table() {
    cout << "Predictive Parsing Table:\n";

    unordered_set<string> terminals;
    for (const auto& row : parsing_table) {
      for (const auto& entry : row.second) {
        terminals.insert(entry.first);
      }
    }

    for (const auto& prod : productions) {
      const string& A = prod.first;
      if (A != "type_params" && A != "type_inheritance") continue;
      cout << A << "\n";

      for (const string& t : terminals) {
        cout << "\t" <<  t << "::";
        if (parsing_table[A].count(t)) {
          cout << A << " -> ";
          for (const string& sym : parsing_table[A][t]) {
            cout << sym << " ";
          }
        }
        cout << "\n";
      }
      cout << endl;
    }
  }

  void print_first_sets() {
    cout << "FIRST sets:\n";
    for (const auto& entry : first) {
      cout << "FIRST(" << entry.first << ") = {";
      for (const string& c : entry.second) {
        cout << c << " ";
      }
      cout << "}\n";
    }
    cout << "\n";
  }

  void print_follow_sets() {
    cout << "FOLLOW sets:\n";
    for (const auto& entry : follow) {
      cout << "FOLLOW(" << entry.first << ") = {";
      for (const string& c : entry.second) {
        cout << c << " ";
      }
      cout << "}\n";
    }
    cout << "\n";
  }

 private:
  unordered_map<string, vector<vector<string>>> productions;
  unordered_map<string, unordered_set<string>> first, follow;
  unordered_map<string, unordered_map<string, vector<string>>> parsing_table;
  unordered_set<string> sync_symbols;
  string start_symbol;
  bool verbose;

  void compute_first() {
    bool changed;
    do {
      changed = false;
      for (const auto& prod : productions) {
        const string& X = prod.first;
        for (const auto& production : prod.second) {
          // if X -> ε is a production, add ε to FIRST(X)
          if (production.size() == 1 && production[0] == epsilon) {
            changed |= first[X].insert(epsilon).second;
            continue;
          }

          // for X -> Y1Y2...Yk
          bool all_epsilon = true;
          for (const auto& Yi : production) {
            if (is_nonterminal(Yi)) {
              // add all non-ε symbols from FIRST(Yi)
              for (const auto& a : first[Yi]) {
                if (a != epsilon) {
                  changed |= first[X].insert(a).second;
                }
              }

              // if Yi doesn't derive ε, stop
              if (!first[Yi].count(epsilon)) {
                all_epsilon = false;
                break;
              }
            } else {
              // Yi is a terminal
              changed |= first[X].insert(Yi).second;
              all_epsilon = false;
              break;
            }
          }

          // if all Yi derive ε, add ε to FIRST(X)
          if (all_epsilon) {
            changed |= first[X].insert(epsilon).second;
          }
        }
      }
    } while (changed);
  }

  void compute_follow() {
    // Place $ in FOLLOW(S)
    follow[start_symbol].insert(eof_symb);

    bool changed;
    do {
      changed = false;
      for (const auto& prod : productions) {
        const string& X = prod.first;
        for (const auto& production : prod.second) {
          const vector<string>& W = production;
          for (size_t i = 0; i < W.size(); i++) {
            const string& B = W[i];
            if (!is_nonterminal(B)) continue;

            // Calculate FIRST of the suffix (β)
            vector<string> beta(W.begin() + i + 1, W.end());
            auto first_beta = compute_firsts(beta);

            // Add FIRST(β) - ε to FOLLOW(B)
            for (const auto& b: first_beta) {
              if (b != epsilon) {
                changed |= follow[B].insert(b).second;
              }
            }

            // If β derives ε, add FOLLOW(X) to FOLLOW(B)
            if (first_beta.count(epsilon) || beta.empty()) {
              for (const string& f : follow[X]) {
                changed |= follow[B].insert(f).second;
              }
            }
          }
        }
      }
    } while (changed);
  }

  unordered_set<string> compute_firsts(const vector<string>& sequence) {
    unordered_set<string> result;

    bool all_epsilon = true;
    for (const auto& symbol : sequence) {
      if (is_nonterminal(symbol)) {
        // Add all non-ε from FIRST(symbol)
        for (const string& a : first[symbol]) {
          if (a != epsilon) {
            result.insert(a);
          }
        }

        // If symbol doesn't derive ε, stop
        if (!first[symbol].count(epsilon)) {
          all_epsilon = false;
          break;
        }
      } else {
        // Terminal - add it and stop
        result.insert(symbol);
        all_epsilon = false;
        break;
      }
    }

    if (all_epsilon) {
      result.insert(epsilon);
    }

    return result;
  }

  bool handle_unexpected_terminal(stack<string>& st, const string& X) {
    st.pop();

    string next = st.top();
    if (is_nonterminal(next) && parsing_table[next].count(X)) {
      return true;
    }

    error(X, "Discarding unexpected token '" + X + "'");
    return true;
  }
  
  bool handle_missing_production(stack<string>& st, vector<string>& w, size_t &ip, const string& X, const string& a) {
    if (parsing_table[X].count(epsilon)) {
      error(X, "Using epsilon production " + X);
      st.pop();
      return true;
    }

    unordered_set<string> sync_set = follow[X];
    sync_set.insert(first[X].begin(), first[X].end());
    sync_set.insert(sync_symbols.begin(), sync_symbols.end());

    if (sync_set.count(a)) {
      error(X, "Skipping nonterminal " + X + " (found sync token '" + a + "')");
      st.pop();
      return true;
    }

    int skips = 0, max_skips = 10;

    while (ip < w.size() && skips < max_skips) {
      if (sync_set.count(w[ip])) {
        error(w[ip], "Found sync token '" + w[ip] + "' after skipping" + to_string(skips) + " tokens");
        return true;
      }
    }

    error(X, "Reached end of input during recovery");
    return false;
  }

  bool is_nonterminal(const string& symbol) {
    return productions.find(symbol) != productions.end();
  }

  string stack_to_string(stack<string> st) {
    string result;
    while (!st.empty()) {
      result = st.top() + " " + result;
      st.pop();
    }
    return result;
  }

  string input_to_string(const vector<string>& input, size_t ip) {
    string result;
    for (size_t i = ip; i < input.size(); i++) {
      result += input[i];
    }
    return result;
  }

  static parse_error error(string x,
                          std::string message) {
    lexer::token token;
    internal::error(token, message, "parser");
    return parse_error(message);
  }
};

};  // namespace gen_parser

}  // namespace hulk

#endif  // HULK_GEN_PARSER_HPP