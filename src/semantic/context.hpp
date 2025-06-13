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
            map<string, protocol> protocols;
            map<string, type> types;
            map<string, method> functions;

            vector<map<string, attribute>> scopes;

            vector<tuple<int, int, string>> infer_errors;
            bool repeat_infer;

            set<ast::binary_op> number_ops = {
                ast::binary_op::PLUS, ast::binary_op::MINUS, ast::binary_op::MULT,
                ast::binary_op::DIVIDE
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

            }; // string <op> string => string 

            context() = default;

            bool create_protocol(const string& protocol_name) {
                if (protocols.find(protocol_name) != protocols.end()) {
                    return false; // Protocol already exists
                }
                protocols[protocol_name] = protocol(protocol_name);
                return true;
            }

            bool protocol_exists(const string& protocol_name) const {
                return protocols.find(protocol_name) != protocols.end();
            }

            protocol& get_protocol(const string& protocol_name) {
                return protocols[protocol_name];
            }

            bool create_type(const string& type_name) {
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

            type* get_lca_type(const type* t1, const type* t2) {
                // todo
                return nullptr; // No common ancestor found
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

            void add_scope() {
                if (scopes.empty())
                    scopes.push_back(map<string, attribute>());
                else
                    scopes.push_back(scopes.back());
            }

            void rollback_scope() {
                if (!scopes.empty()) {
                    scopes.pop_back();
                }
            }

            map<string, attribute>& current_scope() {
                if (scopes.empty()) {
                    // todo handle internal error;
                }
                return scopes.back();
            }

            void add_variable(const string& var_name) {
                if (scopes.empty()) {
                    // todo handle internal error;
                }
                else
                    current_scope()[var_name] = attribute(var_name);
            }

            void add_variable(const string& var_name, const string& type_name) {
                if (scopes.empty()) {
                    // todo handle internal error;
                }
                else
                    current_scope()[var_name] = attribute(var_name, type_name);
            }

            bool variable_exists(const string& var_name) {
                if (scopes.empty()) return false;
                return current_scope().find(var_name) != current_scope().end();
            }

            attribute& get_variable(const string& var_name) {
                return current_scope()[var_name];
            }

            void add_infer_error(int line, int column, const string& message) {
                infer_errors.emplace_back(line, column, message);
            }
        };
    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_HPP