#ifndef HULK_SEMANTIC_SCOPED_VISITOR
#define HULK_SEMANTIC_SCOPED_VISITOR 1

#include "../context"
#include "../../ast/ast"

namespace hulk {
    namespace ast {
        using context = semantic::context;

        void program::scoped_visit(context& ctx) const {
            for (const auto& stmt : statements)
                stmt->scoped_visit(ctx);

            if (main)
                main->scoped_visit(ctx);
        }

        /* Statements */

        void field_stmt::scoped_visit(context& ctx) const {
            if (!type.lexeme.empty() && !ctx.type_exists(type.lexeme)) {
                internal::semantic_error(name.line, name.column, "Type '" + type.lexeme + "' does not exist in field '" + name.lexeme + "'.");
            }
            initializer->scoped_visit(ctx);
        }

        void function_stmt::scoped_visit(context& ctx) const {
            ctx.add_scope();

            if (!return_type.lexeme.empty() && !ctx.type_exists(return_type.lexeme)) {
                internal::semantic_error(return_type.line, return_type.column,
                    "return type '" + return_type.lexeme + "' does not exist in '" + name.lexeme + "'.");
            }

            for (const auto& param : parameters) {
                if (!param.type.lexeme.empty() && !ctx.type_exists(param.type.lexeme)) {
                    internal::semantic_error(param.type.line, param.type.column,
                        "type '" + param.type.lexeme + "' does not exist in parameter '" + param.name.lexeme + "' in function '" + name.lexeme + "'.");
                }
                ctx.add_variable(param.name.lexeme);
            }

            if (body)
                body->scoped_visit(ctx);

            ctx.rollback_scope();
        }

        void class_stmt::scoped_visit(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : parameters) {
                if (!param.type.lexeme.empty() && !ctx.type_exists(param.type.lexeme)) {
                    internal::semantic_error(param.type.line, param.type.column,
                        "Type '" + param.type.lexeme + "' does not exist in parameter '" + param.name.lexeme + "' in class '" + name.lexeme + "'.");
                }
                ctx.add_variable(param.name.lexeme);
            }

            for (const auto& field : fields) {
                field->scoped_visit(ctx);
            }

            ctx.rollback_scope();

            ctx.add_scope();

            ctx.add_variable("self", name.lexeme);

            for (const auto& method : methods) {
                method->scoped_visit(ctx);
            }

            ctx.rollback_scope();
        }

        /* Expressions */

        void block_expr::scoped_visit(context& ctx) const {
            for (const auto& expr : expressions) {
                expr->scoped_visit(ctx);
            }
        }

        void call_expr::scoped_visit(context& ctx) const {
            if (object.has_value()) {
                object.value()->scoped_visit(ctx);
            }
            else {
                if (!ctx.function_exists(callee.lexeme)) {
                    internal::semantic_error(callee.line, callee.column,
                        "Function '" + callee.lexeme + "' does not exist.");
                }

                auto& func = ctx.get_function(callee.lexeme);
                if (func.params.size() != arguments.size()) {
                    internal::semantic_error(callee.line, callee.column,
                        "Function '" + callee.lexeme + "' expects " + std::to_string(func.params.size()) +
                        " arguments, but got " + std::to_string(arguments.size()) + ".");
                }

                for (const auto& arg : arguments) {
                    arg->scoped_visit(ctx);
                }
            }
        }

        void let_expr::scoped_visit(context& ctx) const {
            for (const auto& assignment : assignments) {
                ctx.add_scope();
                assignment->scoped_visit(ctx);
            }

            body->scoped_visit(ctx);

            for (const auto& assignment : assignments) {
                ctx.rollback_scope();
            }
        }

        void declaration_expr::scoped_visit(context& ctx) const {
            if (!type.lexeme.empty() && !ctx.type_exists(type.lexeme)) {
                internal::semantic_error(name.line, name.column,
                    "Type '" + type.lexeme + "' does not exist in declaration '" + name.lexeme + "'.");
            }

            if (value) {
                value->scoped_visit(ctx);
            }

            ctx.add_variable(name.lexeme, type.lexeme);
        }

        void assign_expr::scoped_visit(context& ctx) const {
            variable->scoped_visit(ctx);

            value->scoped_visit(ctx);
        }

        void if_expr::scoped_visit(context& ctx) const {
            condition->scoped_visit(ctx);

            then_branch->scoped_visit(ctx);

            for (const auto& elif : elif_branchs) {
                elif.first->scoped_visit(ctx);
                elif.second->scoped_visit(ctx);
            }

            if (else_branch.has_value()) {
                else_branch.value()->scoped_visit(ctx);
            }
        }

        void while_expr::scoped_visit(context& ctx) const {
            condition->scoped_visit(ctx);
            body->scoped_visit(ctx);
        }

        void for_expr::scoped_visit(context& ctx) const {
            if (!var_type.lexeme.empty() && !ctx.type_exists(var_type.lexeme)) {
                internal::semantic_error(var_name.line, var_name.column,
                    "Type '" + var_type.lexeme + "' does not exist in for loop variable '" + var_name.lexeme + "'.");
            }

            iterable->scoped_visit(ctx);

            ctx.add_scope();

            ctx.add_variable(var_name.lexeme);

            if (body) {
                body->scoped_visit(ctx);
            }

            ctx.rollback_scope();
        }

        void binary_expr::scoped_visit(context& ctx) const {
            left->scoped_visit(ctx);
            right->scoped_visit(ctx);
        }

        void unary_expr::scoped_visit(context& ctx) const {
            expression->scoped_visit(ctx);
        }

        void var_expr::scoped_visit(context& ctx) const {
            if (object.has_value()) {
                object.value()->scoped_visit(ctx);
            }
            else {
                if (!ctx.variable_exists(name.lexeme)) {
                    internal::semantic_error(name.line, name.column,
                        "variable '" + name.lexeme + "' does not exist.");
                }
            }
        }

        void new_expr::scoped_visit(context& ctx) const {
            if (!type_name.lexeme.empty() && !ctx.type_exists(type_name.lexeme)) {
                internal::semantic_error(type_name.line, type_name.column,
                    "Type '" + type_name.lexeme + "' does not exist.");
            }

            auto& type = ctx.get_type(type_name.lexeme);

            if (type.params.size() != arguments.size()) {
                internal::semantic_error(type_name.line, type_name.column,
                    "Type '" + type_name.lexeme + "' expects " + std::to_string(type.params.size()) +
                    " arguments, but got " + std::to_string(arguments.size()) + ".");
            }

            for (const auto& arg : arguments) {
                arg->scoped_visit(ctx);
            }
        }

    } // namespace ast
} // namespace hulk

#endif