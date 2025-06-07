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

        struct attribute {
            string name;
            type* attr_type;

            attribute(const string& attr_name)
                : name(attr_name), attr_type(nullptr) {
            }

            attribute(const string& attr_name, type& attr_type)
                : name(attr_name), attr_type(&attr_type) {
            }
        };

        struct method {
            string name;
            type* return_type;
            vector<attribute> params;

            method() = default;

            method(const string& method_name)
                : name(method_name), return_type(nullptr) {
            }

            method(const string& method_name, type& return_type)
                : name(method_name), return_type(&return_type) {
            }

            // Define a parameter for the method
            bool add_parameter(const string& param_name, type& param_type) {
                for (const auto& param : params)
                    if (param.name == param_name)
                        return false; // Parameter already exists

                params.emplace_back(param_name, param_type);
                return true;
            }

            bool add_parameter(const string& param_name) {
                for (const auto& param : params)
                    if (param.name == param_name)
                        return false; // Parameter already exists

                params.emplace_back(param_name);
                return true;
            }
        };

        struct type {
            string name;
            vector<attribute> fields;
            vector<method> methods;
            vector<string> parents;

            type(const string& type_name = "")
                : name(type_name) {
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