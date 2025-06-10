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
            bool add_param(const string& param_name, const string& param_type = "") {
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
            vector<type> parents;

            type(const string& type_name = "")
                : name(type_name) {
            }

            bool operator==(const type& other) const {
                return name == other.name;
            }

            bool operator!=(const type& other) const {
                return !(*this == other);
            }

            bool operator<=(const type& other) const {
                if (*this == other) return true;
                for (const auto& parent : parents) {
                    if (parent <= other)
                        return true;
                }
                return false;
            }

            bool add_param(const string& param_name, const string& param_type = "") {
                for (const auto& param : params)
                    if (param.name == param_name)
                        return false; // Parameter already exists

                params.emplace_back(param_name, param_type);
                return true;
            }

            bool add_parent(const type& parent_type) {
                for (const auto& parent : parents)
                    if (parent == parent_type)
                        return false; // Parent already exists

                parents.push_back(parent_type);
                return true;
            }

            bool add_field(const string& field_name, const string& field_type = "") {
                for (const auto& field : fields)
                    if (field.name == field_name)
                        return false; // Attribute already exists

                fields.emplace_back(field_name, field_type);
                return true;
            }

            bool has_field(const string& field_name) const {
                for (const auto& field : fields) {
                    if (field.name == field_name) {
                        return true;
                    }
                }
                return false;
            }

            attribute& get_field(const string& field_name) {
                for (auto& field : fields) {
                    if (field.name == field_name) {
                        return field;
                    }
                }
                // warn: this is dangerous, make sure to check with has_field first
                return *(new attribute());
            }

            bool add_method(const method& m) {
                for (const auto& method : methods)
                    if (method.name == m.name)
                        return false; // Method already exists

                methods.push_back(m);
                return true;
            }

            bool has_method(const string& method_name) const {
                for (const auto& method : methods) {
                    if (method.name == method_name) {
                        return true;
                    }
                }
                return false;
            }

            method& get_method(const string& method_name) {
                for (auto& method : methods) {
                    if (method.name == method_name) {
                        return method;
                    }
                }
                // warn: this is dangerous, make sure to check with has_method first
                return *(new method());
            }

        };
    }
}

#endif // HULK_SEMANTIC_TYPE_HPP