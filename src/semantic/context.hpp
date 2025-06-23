#ifndef HULK_SEMANTIC_CONTEXT_HPP
#define HULK_SEMANTIC_CONTEXT_HPP 1

#include <string>
#include <vector>
#include <map>
#include <set>
#include "type.hpp"
#include "../ast/ast"
using namespace std;

namespace hulk {
    namespace semantic {

        struct context {
            map<string, type> types;
            map<string, method> functions;

            map<string, vector<string>> variable_scope;
            string self;

            vector<tuple<int, int, string>> infer_errors;
            bool repeat_infer;

            set<ast::binary_op> number_ops = {
                ast::binary_op::PLUS, ast::binary_op::MINUS, ast::binary_op::MULT,
                ast::binary_op::DIVIDE, ast::binary_op::MODULE, ast::binary_op::EXPONENT
            }; //  number <op> number => number

            set<ast::binary_op> boolean_ops = {
                ast::binary_op::OR, ast::binary_op::AND
            }; // boolean <op> boolean => boolean

            set<ast::binary_op> comparison_ops = {
                ast::binary_op::GREATER, ast::binary_op::GREATER_EQUAL,
                ast::binary_op::LESS, ast::binary_op::LESS_EQUAL
            }; // number <op> number => boolean

            set<ast::binary_op> equality_ops = {
                ast::binary_op::EQUAL_EQUAL, ast::binary_op::NOT_EQUAL
            }; // any <op> any => boolean

            set<ast::binary_op> string_ops = {
                ast::binary_op::CONCAT, ast::binary_op::CONCAT_DOBLE
            }; // string <op> string => string 

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
                types[type_name] = type(type_name);
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

            void add_infer_error(int line, int column, const string& message) {
                infer_errors.emplace_back(line, column, message);
            }
        };
        
    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_HPP