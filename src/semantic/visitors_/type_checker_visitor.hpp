#ifndef TYPE_CHECKER_VISITOR_HPP
#define TYPE_CHECKER_VISITOR_HPP

#include "context.hpp"
#include "../ast/ast"

namespace hulk {
    namespace ast {
        using context = semantic::context;
        using type = semantic::type;
        using method = semantic::method;

        void program::type_check(context& ctx) const {
            for (const auto& stmt : statements)
                stmt->type_check(ctx);

            if (main)
                main->type_check(ctx);
        }

        void class_stmt::type_check(context& ctx) const {
            type& this_type = ctx.get_type(name.lexeme);

            ctx.add_scope();

            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            if (super_class.has_value()) {
                type& parent_type = ctx.get_type(super_class.value()->name.lexeme);
                auto& parent_args = super_class.value()->init;

                for (int i = 0; i < parent_args.size(); ++i) {
                    string arg_type = parent_args[i]->type_check(ctx);
                    if (ctx.get_type(arg_type) <= ctx.get_type(parent_type.params[i].attr_type));
                    else {
                        internal::semantic_error(super_class.value()->name.line, super_class.value()->name.column,
                            "type '" + name.lexeme + "' expects argument " + std::to_string(i + 1) +
                            " of type '" + parent_type.params[i].attr_type + "', but got '" + arg_type + "'");
                    }
                }
            }

            for (const auto& field : fields) {
                field->type_check(ctx);
            }

            ctx.rollback_scope();

            ctx.add_scope();

            ctx.add_variable("self", name.lexeme);

            for (const auto& method : methods)
                method->type_check(ctx);

            ctx.rollback_scope();
        }

        void function_stmt::type_check(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            if (body)
                body->type_check(ctx);

            ctx.rollback_scope();
        }

        void field_stmt::type_check(context& ctx) const {
            string value_type = initializer->type_check(ctx);

            if (ctx.get_type(value_type) <= ctx.get_type(type.lexeme));
            else {
                internal::semantic_error(name.line, name.column,
                    "field '" + name.lexeme + "' expects type '" + type.lexeme +
                    "', but got '" + value_type + "'");
            }
        }

        // Expressions

        string block_expr::type_check(context& ctx) {
            string last_type = "Object";

            for (int i = 0; i < expressions.size(); ++i) {
                string expr_type = expressions[i]->type_check(ctx);

                if (i == expressions.size() - 1)
                    last_type = expr_type;
            }

            return last_type;
        }

        string binary_expr::type_check(context& ctx) {
            string left_type = left->type_check(ctx);
            string right_type = right->type_check(ctx);

            if (ctx.number_ops.count(op)) {
                if (left_type != "Number")
                    internal::semantic_error(token.line, token.column, token.lexeme + " left operand is not a number");
                if (right_type != "Number")
                    internal::semantic_error(token.line, token.column, token.lexeme + " right operand is not a number");

                return "Number";
            }

            if (ctx.boolean_ops.count(op)) {
                if (left_type != "Boolean")
                    internal::semantic_error(token.line, token.column, token.lexeme + " left operand is not a boolean");
                if (right_type != "Boolean")
                    internal::semantic_error(token.line, token.column, token.lexeme + " right operand is not a boolean");

                return "Boolean";
            }

            if (ctx.comparison_ops.count(op)) {
                if (left_type != "Number")
                    internal::semantic_error(token.line, token.column, token.lexeme + " left operand is not a number");
                if (right_type != "Number")
                    internal::semantic_error(token.line, token.column, token.lexeme + " right operand is not a number");

                return "Boolean";
            }

            if (ctx.equality_ops.count(op)) {
                return "Boolean";
            }

            if (ctx.string_ops.count(op)) {
                if (left_type != "String")
                    internal::semantic_error(token.line, token.column, token.lexeme + " left operand is not a string");
                if (right_type != "String")
                    internal::semantic_error(token.line, token.column, token.lexeme + " right operand is not a string");

                return "String";
            }

            // unknown operator
            internal::error(token, "unknown operator: " + token.lexeme);
            return "Object";
        }

        string unary_expr::type_check(context& ctx) {
            string expr_type = expression->type_check(ctx);

            if (op == unary_op::MINUS) {
                if (expr_type != "Number")
                    internal::semantic_error(token.line, token.column, token.lexeme + " operand is not a number");

                return "Number";
            }

            if (op == unary_op::NOT) {
                if (expr_type != "Boolean")
                    internal::semantic_error(token.line, token.column, token.lexeme + " operand is not a boolean");

                return "Boolean";
            }

            // unknown operator
            internal::error(token, "unknown operator: " + token.lexeme);
            return "Object";
        }

        string literal_expr::type_check(context& ctx) {
            switch (value.index()) {
            case 0:
                return "Object";
            case 1:
                return "String";
            case 2:
                return "Number";
            case 3:
                return "Boolean";
            default:
                internal::error(0, 0, "unknown literal type" + std::to_string(value.index()));
                return "Object";
            }
        }

        string new_expr::type_check(context& ctx) {
            int args_count = arguments.size();
            type current = ctx.get_type(type_name.lexeme);

            while (current.params.size() == 0) {
                if (current.parent)
                    current = ctx.get_type(current.parent->name);
                else {
                    internal::semantic_error(type_name.line, type_name.column, "type '" + type_name.lexeme + "' does not have parameters");
                    return "Object";
                }
            }

            if (args_count != current.params.size()) {
                internal::semantic_error(type_name.line, type_name.column,
                    "type '" + type_name.lexeme + "' expects " + std::to_string(current.params.size()) +
                    " arguments in type '" + current.name + "', but got " + std::to_string(args_count));
                return "Object";
            }

            for (int i = 0; i < args_count; ++i) {
                string arg_type = arguments[i]->type_check(ctx);
                if (ctx.get_type(arg_type) <= ctx.get_type(current.params[i].attr_type));
                else {
                    internal::semantic_error(type_name.line, type_name.column,
                        "type '" + type_name.lexeme + "' expects argument " + std::to_string(i + 1) +
                        " of type '" + current.params[i].attr_type + "', but got '" + arg_type + "'");
                }
            }

            return type_name.lexeme;
        }

        string call_expr::type_check(context& ctx) {
            if (object.has_value()) {
                string obj_type = object.value()->type_check(ctx);
                auto& obj = ctx.get_type(obj_type);

                if (obj.has_method(calle.lexeme)) {
                    auto& method = obj.get_method(calle.lexeme);
                    if (method.params.size() != arguments.size()) {
                        internal::semantic_error(calle.line, calle.column,
                            "Method '" + calle.lexeme + "' expects " + std::to_string(method.params.size()) +
                            " arguments, but got " + std::to_string(arguments.size()) + ".");
                    }

                    for (size_t i = 0; i < arguments.size(); ++i) {
                        string arg_type = arguments[i]->type_check(ctx);
                        if (ctx.get_type(arg_type) <= ctx.get_type(method.params[i].attr_type));
                        else {
                            internal::semantic_error(calle.line, calle.column,
                                "Method '" + calle.lexeme + "' expects argument " + std::to_string(i + 1) +
                                " of type '" + method.params[i].attr_type + "', but got '" + arg_type + "'");
                        }
                    }

                    return method.return_type;
                }
                else {
                    internal::semantic_error(calle.line, calle.column,
                        "Object of type '" + obj.name + "' does not have method '" + calle.lexeme + "'");
                }

                return "Object"; // If method does not exist, return Object type
            }
            else {
                auto& func = ctx.get_function(calle.lexeme);
                if (func.params.size() != arguments.size()) {
                    internal::semantic_error(calle.line, calle.column,
                        "Function '" + calle.lexeme + "' expects " + std::to_string(func.params.size()) +
                        " arguments, but got " + std::to_string(arguments.size()) + ".");
                }

                for (size_t i = 0; i < arguments.size(); ++i) {
                    string arg_type = arguments[i]->type_check(ctx);
                    if (ctx.get_type(arg_type) <= ctx.get_type(func.params[i].attr_type));
                    else {
                        internal::semantic_error(calle.line, calle.column,
                            "Function '" + calle.lexeme + "' expects argument " + std::to_string(i + 1) +
                            " of type '" + func.params[i].attr_type + "', but got '" + arg_type + "'");
                    }
                }

                return func.return_type;
            }
        }

        string let_expr::type_check(context& ctx) {
            for (const auto& assignment : assignments) {
                ctx.add_scope();
                assignment->type_check(ctx);
            }

            string body_type = body->type_check(ctx);

            for (const auto& assignment : assignments) {
                ctx.rollback_scope();
            }

            return body_type;
        }

        string declaration_expr::type_check(context& ctx) {
            string value_type = value->type_check(ctx);
            if (ctx.get_type(value_type) <= ctx.get_type(type.lexeme));
            else {
                internal::semantic_error(name.line, name.column,
                    "type '" + type.lexeme + "' does not match value type '" + value_type + "'");
            }

            ctx.add_variable(name.lexeme, type.lexeme);
            return type.lexeme;
        }

        string assign_expr::type_check(context& ctx) {
            string var_type = variable->type_check(ctx);

            string value_type = value->type_check(ctx);

            if (ctx.get_type(value_type) <= ctx.get_type(var_type));
            else {
                internal::semantic_error(type.line, type.column,
                    "type '" + var_type + "' does not match value type '" + value_type + "'");
            }

            return var_type;
        }

        string if_expr::type_check(context& ctx) {
            string cond_type = condition->type_check(ctx);
            if (ctx.get_type(cond_type) == ctx.get_type("Boolean"));
            else {
                internal::semantic_error(0, 0,
                    "condition is not a boolean, got '" + cond_type + "'");
            }

            string then_type = then_branch->type_check(ctx);

            for (const auto& elif : elif_branchs) {
                string elif_cond_type = elif.first->type_check(ctx);
                if (ctx.get_type(elif_cond_type) == ctx.get_type("Boolean"));
                else {
                    internal::semantic_error(0, 0,
                        "elif condition is not a boolean, got '" + elif_cond_type + "'");
                }

                string elif_type = elif.second->type_check(ctx);
                then_type = ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(elif_type))->name;
            }

            if (else_branch.has_value()) {
                string else_type = else_branch.value()->type_check(ctx);
                then_type = ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(else_type))->name;
            }

            return then_type;
        }

        string while_expr::type_check(context& ctx) {
            string cond_type = condition->type_check(ctx);

            if (ctx.get_type(cond_type) == ctx.get_type("Boolean"));
            else {
                internal::semantic_error(0, 0,
                    "condition is not a boolean, got '" + cond_type + "'");
            }

            string body_type = body->type_check(ctx);

            return body_type;
        }

        string for_expr::type_check(context& ctx) {
            string iterable_type = iterable->type_check(ctx);
            auto& iter_type = ctx.get_type(iterable_type);

            if (iter_type <= ctx.get_type("Iterable")) {
                auto& current_method = iter_type.get_method("current");

                if (ctx.get_type(current_method.return_type) <= ctx.get_type(var_type.lexeme));
                else {
                    internal::semantic_error(var_name.line, var_name.column,
                        "type '" + var_type.lexeme + "' does not match iterable's current type '" + current_method.return_type + "'");
                }
            }
            else {
                internal::semantic_error(var_name.line, var_name.column,
                    "iterable is not an Iterable, got '" + iterable_type + "'");
            }

            ctx.add_scope();

            ctx.add_variable(var_name.lexeme, var_type.lexeme);

            string body_type = body->type_check(ctx);

            ctx.rollback_scope();

            return body_type;
        }

        string var_expr::type_check(context& ctx) {
            if (object.has_value()) {
                string _type = object.value()->type_check(ctx);
                auto& obj_type = ctx.get_type(_type);

                if (obj_type.has_field(name.lexeme)) {
                    auto& field = obj_type.get_field(name.lexeme);
                    return field.attr_type;
                }
                else {
                    internal::semantic_error(name.line, name.column,
                        "object of type '" + _type + "' does not have field '" + name.lexeme + "'");
                }
            }
            else {
                auto& var = ctx.get_variable(name.lexeme);
                return var.attr_type;
            }
        }

    } // namespace ast
} // namespace hulk

#endif //TYPE_CHECKER_VISITOR_HPP