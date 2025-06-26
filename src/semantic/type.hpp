#ifndef HULK_SEMANTIC_TYPE_HPP
#define HULK_SEMANTIC_TYPE_HPP 1

#include <iostream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

namespace hulk {
    namespace semantic {

        // Forward declarations
        struct attribute;
        struct method;
        struct type;

        using type_ptr = shared_ptr<type>;

        struct attribute {
            string name;
            type_ptr attr_type;

            attribute() = default;

            attribute(const string& attr_name, const type_ptr& attr_type = nullptr)
                : name(attr_name), attr_type(attr_type) {
            }
        };

        struct method {
            string name;
            type_ptr return_type;
            vector<attribute> params;

            method() = default;

            method(const string& method_name, const type_ptr& return_type = nullptr)
                : name(method_name), return_type(return_type) {
            }

            // Define a parameter for the method
            bool add_param(const string& param_name, const type_ptr& param_type = nullptr) {
                for (const auto& param : params)
                    if (param.name == param_name)
                        return false; // Parameter already exists

                params.emplace_back(param_name, param_type);
                return true;
            }

            attribute& get_param(const string& param_name) {
                for (auto& param : params)
                    if (param.name == param_name)
                        return param;

                return *(new attribute()); // warn: this is dangerous
            }
        };

        struct type {
            string name;
            vector<attribute> params;
            vector<attribute> fields;
            vector<method> methods;
            type_ptr parent;
            bool is_protocol;

            type(const string& type_name = "", bool _is_protocol = false)
                : name(type_name), is_protocol(_is_protocol) {
            }

            bool operator==(const type& other) const {
                return name == other.name;
            }

            bool operator!=(const type& other) const {
                return !(*this == other);
            }

            bool operator<=(const type& other) const {
                if (*this == other) return true;

                if (other.is_protocol) {
                    bool conforms = true;

                    for (const auto& method : other.methods) {
                        if (!has_method(method.name))
                            return false; // If any method in 'proto' is not in 'this', return false

                        auto& this_method = get_method(method.name);
                        conforms &= this_method.return_type <= method.return_type;

                        if (this_method.params.size() != method.params.size())
                            return false; // If the number of parameters doesn't match, return false

                        for (size_t i = 0; i < this_method.params.size(); ++i)
                            conforms &= method.params[i].attr_type <= this_method.params[i].attr_type;
                    }

                    return conforms;
                }

                if (*parent <= other)
                    return true;

                return false;
            }

            bool add_param(const string& param_name, const type_ptr& param_type = nullptr) {
                for (const auto& param : params)
                    if (param.name == param_name)
                        return false; // Parameter already exists

                params.emplace_back(param_name, param_type);
                return true;
            }

            bool add_parent(type_ptr& parent_type) {
                if (parent)
                    return false; // Parent already exists

                parent = parent_type;
                return true;
            }

            bool add_field(const string& field_name, const type_ptr& field_type = nullptr) {
                for (const auto& field : fields)
                    if (field.name == field_name)
                        return false; // Field already exists

                fields.emplace_back(field_name, field_type);
                return true;
            }

            bool has_field(const string& field_name) const {
                for (const auto& field : fields)
                    if (field.name == field_name)
                        return true;

                return false;
            }

            attribute& get_field(const string& field_name) {
                for (auto& field : fields)
                    if (field.name == field_name)
                        return field;

                return *(new attribute()); // warn: this is dangerous, make sure to check with has_field first
            }

            bool add_method(const method& m) {
                for (const auto& method : methods)
                    if (method.name == m.name)
                        return false; // Method already exists

                methods.push_back(m);
                return true;
            }

            bool has_method(const string& method_name) const {
                for (const auto& method : methods)
                    if (method.name == method_name)
                        return true;

                if (parent)
                    return parent->has_method(method_name);

                return false;
            }

            method& get_method(const string& method_name) {
                for (auto& method : methods)
                    if (method.name == method_name)
                        return method;

                if (parent)
                    return parent->get_method(method_name);

                return *(new method()); // warn: this is dangerous, make sure to check with has_method first
            }

            const method& get_method(const string& method_name) const {
                for (const auto& method : methods)
                    if (method.name == method_name)
                        return method;

                if (parent)
                    return parent->get_method(method_name);

                return *(new method()); // warn: this is dangerous, make sure to check with has_method first
            }
        };

    }
}

#endif // HULK_SEMANTIC_TYPE_HPP