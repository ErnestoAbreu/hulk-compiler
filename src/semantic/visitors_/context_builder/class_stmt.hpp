#ifndef HULK_SEMANTIC_CONTEXT_BUILDER_CLASS_STMT_HPP
#define HULK_SEMANTIC_CONTEXT_BUILDER_CLASS_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {
        using type_ptr = std::shared_ptr<semantic::type>;

        void class_stmt::context_builder_visit(semantic::context& ctx) {
            // Create the type in the context
            if (!ctx.create_type(name.lexeme)) {
                internal::error(name, "Type already exists.");
                return;
            }

            // Get the type from the context
            auto& type = ctx.get_type(name.lexeme);

            // Add constructor params
            for (const auto& param : parameters) {
                bool error = false;
                if (param.type.lexeme.empty()) {
                    error = !type.add_param(param.name.lexeme);
                }
                else {
                    if (!ctx.type_exists(param.type.lexeme)) {
                        internal::error(param.type, "Type does not exist in parameter '" + param.name.lexeme + "' in class '" + name.lexeme + "'.");
                    }
                    else
                        error = !type.add_param(param.name.lexeme, std::make_shared<semantic::type>(ctx.get_type(param.type.lexeme)));
                }
                if (error) {
                    internal::error(param.name, "parameter already exists in class '" + name.lexeme + "'.");
                }
            }

            // Add parent class
            if (super_class) {
                string super_class_name = super_class->get()->name.lexeme;
                if (!ctx.type_exists(super_class_name)) {
                    internal::error(super_class->get()->name, "parent type does not exist, in class '" + name.lexeme + "'.");
                }
                else if (super_class_name == "Number" || super_class_name == "String" || super_class_name == "Boolean") {
                    internal::error(super_class->get()->name, "parent type cannot be a builtin type.");
                }
                else {
                    type_ptr super_type = std::make_shared<semantic::type>(ctx.get_type(super_class_name));
                    type.add_parent(super_type);
                }

                if (super_class->get()->init.empty()) {
                    if (!parameters.empty()) {
                        internal::error(super_class->get()->name, "parent class '" + super_class_name + "' must have an initializer if the child class has parameters.");
                    }
                    else {
                        // If the parent class has no initializer, we can add its parameters to the child class
                        auto& super_type = ctx.get_type(super_class_name);
                        for (const auto& param : super_type.params) {
                            type.add_param("_" + param.name, param.attr_type);

                            parameters.push_back(parameter(
                                lexer::token(lexer::token_type::IDENTIFIER, "_" + param.name, "", name.line, name.column),
                                param.attr_type ? lexer::token(lexer::token_type::IDENTIFIER, param.attr_type->name, "", name.line, name.column) : lexer::token()
                            ));

                            expr_ptr init_expr = std::make_unique<var_expr>(std::nullopt, lexer::token(lexer::token_type::IDENTIFIER, "_" + param.name, "", name.line, name.column));
                            super_class->get()->init.push_back(std::move(init_expr));
                        }
                    }
                }
            }
            else {
                // If no parent class, add Object as the default parent
                auto object_ptr = std::make_shared<semantic::type>(ctx.get_type("Object"));
                type.add_parent(object_ptr);
                super_class = std::optional<super_item_ptr>(
                    std::make_shared<super_item>(
                        lexer::token(lexer::token_type::IDENTIFIER, "Object", "", name.line, name.column), std::vector<expr_ptr>()
                    )
                );
            }

            // Add fields to the type
            for (const auto& field : fields) {
                bool error = false;
                if (field->type.lexeme.empty()) {
                    error = !type.add_field(field->name.lexeme);
                }
                else {
                    if (!ctx.type_exists(field->type.lexeme)) {
                        internal::error(field->type, "Type does not exist in field '" + field->name.lexeme + "' in class '" + name.lexeme + "'.");
                    }
                    else
                        error = !type.add_field(field->name.lexeme, std::make_shared<semantic::type>(ctx.get_type(field->type.lexeme)));
                }
                if (error) {
                    internal::error(field->name, "field already exists in class '" + name.lexeme + "'.");
                }
            }

            // Add methods to the type
            for (const auto& meth : methods) {
                semantic::method f;
                if (meth->return_type.lexeme.empty()) {
                    f = semantic::method(meth->name.lexeme);
                }
                else {
                    if (!ctx.type_exists(meth->return_type.lexeme)) {
                        internal::error(meth->return_type, "return type does not exist in method '" + meth->name.lexeme + "' in class '" + name.lexeme + "'.");
                    }
                    else
                        f = semantic::method(meth->name.lexeme, std::make_shared<semantic::type>(ctx.get_type(meth->return_type.lexeme)));
                }

                for (const auto& param : meth->parameters) {
                    bool error = false;
                    if (param.type.lexeme.empty()) {
                        error = !f.add_param(param.name.lexeme);
                    }
                    else {
                        if (!ctx.type_exists(param.type.lexeme)) {
                            internal::error(param.type, "parameter type does not exist in method '" + meth->name.lexeme + "' in class '" + name.lexeme + "'.");
                        }
                        else
                            error = !f.add_param(param.name.lexeme, std::make_shared<semantic::type>(ctx.get_type(param.type.lexeme)));
                    }

                    if (error) {
                        internal::error(param.name, "parameter already exists in method '" + meth->name.lexeme + "' in class '" + name.lexeme + "'.");
                    }
                }

                if (!type.add_method(f)) {
                    internal::error(meth->name, "method already exists in class '" + name.lexeme + "'.");
                }
            };
        }

    }  // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_BUILDER_CLASS_STMT_HPP