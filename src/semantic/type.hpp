#ifndef HULK_SEMANTIC_TYPE_HPP
#define HULK_SEMANTIC_TYPE_HPP

#include <string>
#include <vector>
using namespace std;

namespace hulk {
    namespace semantic {

        // Forward declarations
        struct type;
        struct attribute;
        struct method;

        struct typed_method {
            string name;
            string return_type;
            vector<string> param_types;
        };

        struct type_rules {
            string closed_type;
            vector<string> must_equal;
            vector<typed_method> must_have;
        };

        struct attribute {
            string name;
            string attr_type;

            attribute() = default;

            attribute(const string& attr_name, const string& attr_type = "")
                : name(attr_name), attr_type(attr_type) {
            }
        };

        struct method {
            string name;
            string return_type;
            vector<attribute> params;

            method() = default;

            method(const string& method_name, const string& return_type = "")
                : name(method_name), return_type(return_type) {
            }

            // Define a parameter for the method
            bool add_parameter(const string& param_name, const string& param_type = "") {
                for (const auto& param : params)
                    if (param.name == param_name)
                        return false; // Parameter already exists

                params.emplace_back(param_name, param_type);
                return true;
            }
        };

        struct type {
            string name;
            vector<attribute> params;
            vector<attribute> fields;
            vector<method> methods;
            vector<string> parents;

            type(const string& type_name = "")
                : name(type_name) {
            }

            bool add_param(const attribute& param) {
                for (const auto& p : params)
                    if (p.name == param.name)
                        return false; // Parameter already exists

                params.push_back(param);
                return true;
            }

            bool add_attribute(const attribute& attr) {
                for (const auto& field : fields)
                    if (field.name == attr.name)
                        return false; // Attribute already exists

                fields.push_back(attr);
                return true;
            }

            bool add_method(const method& m) {
                for (const auto& method : methods)
                    if (method.name == m.name)
                        return false; // Method already exists

                methods.push_back(m);
                return true;
            }

            bool add_parent(const string& parent_name) {
                for (const auto& parent : parents)
                    if (parent == parent_name)
                        return false; // Parent already exists

                parents.push_back(parent_name);
                return true;
            }
        };
    }
}

#endif // HULK_SEMANTIC_TYPE_HPP