#ifndef HULK_SEMANTIC_CONTEXT_HPP
#define HULK_SEMANTIC_CONTEXT_HPP 1

#include <string>
#include <vector>
#include <map>
#include <set>
#include "type.hpp"
using namespace std;

namespace hulk {
    namespace semantic {

        struct context {
            map<string, type> types;
            map<string, method> functions;

            map<string, vector<string>> variable_scope;
            string self;

            context() = default;

            bool create_protocol(const string& protocol_name) {
                if (!create_type(protocol_name, true))
                    return false; // Type creation failed
                return true;
            }

            bool is_protocol(const string& protocol_name) {
                if (types[protocol_name].is_protocol)
                    return true;
                return false;
            }

            bool create_type(const string& type_name, bool is_protocol = false) {
                if (types.find(type_name) != types.end()) {
                    return false; // Type already exists
                }
                types[type_name] = type(type_name, is_protocol);
                return true;
            }

            bool type_exists(const string& type_name) const {
                return types.find(type_name) != types.end();
            }

            // Use type_exists to check if the type exists before getting it
            type& get_type(const string& type_name) {
                return types[type_name];
            }

            type* get_lca_type(type* t1, type* t2) {
                if (*t1 <= *t2)
                    return t2;
                if (*t2 <= *t1)
                    return t1;
                // Find the least common ancestor (LCA) of two types

                type_ptr lca = t1->parent;
                while (lca) {
                    if (*t2 <= *lca) {
                        return lca.get(); // Found LCA
                    }
                    lca = lca->parent;
                }

                return &get_type("Object"); // No common ancestor found
            }

            bool create_function(const string& func_name) {
                if (functions.find(func_name) != functions.end()) {
                    return false; // Function already exists
                }
                functions[func_name] = method(func_name);
                return true;
            }

            bool function_exists(const string& func_name) const {
                return functions.find(func_name) != functions.end();
            }

            method& get_function(const string& func_name) {
                return functions[func_name];
            }

            void add_variable(const string& var_name, const string& type_name = "") {
                variable_scope[var_name].push_back(type_name);
            }

            void rollback_variable(const string& var_name) {
                variable_scope[var_name].pop_back();
            }

            bool variable_exists(const string& var_name) {
                if (variable_scope[var_name].empty())
                    return false;
                return true;
            }

            string& get_variable_type(const string& var_name) {
                return variable_scope[var_name].back();
            }
        };
        
    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_HPP