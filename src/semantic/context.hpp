#ifndef HULK_SEMANTIC_CONTEXT_HPP
#define HULK_SEMANTIC_CONTEXT_HPP

#include <string>
#include <vector>
#include <map>
#include "type.hpp"
using namespace std;

namespace hulk {
    namespace semantic {
        struct context {
            map<string, type> types;
            map<string, method> functions;

            context() = default;

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
        };
    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_HPP