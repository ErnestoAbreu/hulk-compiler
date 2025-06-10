#ifndef HULK_SEMANTIC_SCOPED_VISITOR
#define HULK_SEMANTIC_SCOPED_VISITOR

#include "context.hpp"
#include "../ast/node.hpp"

namespace hulk {
    namespace ast {
        using context = semantic::context;

        void program::scoped_visit(context& ctx) const {
            for (const auto& stmt : statements) {
                stmt->scoped_visit(ctx);
            }
            if (main)
                main->scoped_visit(ctx);
        }

        /* Statements */

        void def_field::scoped_visit(context& ctx) const {
            if (!var_type.empty() && !ctx.type_exists(var_type)) {
                // todo handle error: type does not exist
            }
            value->scoped_visit(ctx);
        }

        void def_function::scoped_visit(context& ctx) const {
            ctx.add_scope();

            if (!return_type.empty() && !ctx.type_exists(return_type)) {
                // todo handle error: return type does not exist
            }

            for (const auto& param : params) {
                if (!param.type.empty() && !ctx.type_exists(param.type)) {
                    // todo handle error: type does not exist
                }
                ctx.add_variable(param.id);
            }

            if (body)
                body->scoped_visit(ctx);

            ctx.rollback_scope();
        }

        void def_type::scoped_visit(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : params) {
                if (!param.type.empty() && !ctx.type_exists(param.type)) {
                    // todo handle error: type does not exist
                }
                ctx.add_variable(param.id);
            }

            for (const auto& field : fields) {
                field->scoped_visit(ctx);
            }

            ctx.rollback_scope();

            ctx.add_scope();

            // todo: handle parent scope merging

            for (const auto& field : fields) {
                ctx.add_variable("self", id);
            }

            for (const auto& method : methods) {
                method->scoped_visit(ctx);
            }

            ctx.rollback_scope();
        }

        /* Expressions */

        void block_expr::scoped_visit(context& ctx) const {
            for (const auto& expr : exprs) {
                expr->scoped_visit(ctx);
            }
        }

        void call_expr::scoped_visit(context& ctx) const {
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string method = id.substr(id.find('.') + 1);

                if (!ctx.variable_exists(name)) {
                    // todo handle error: variable does not exist in this scope
                }
                else {
                    auto var_type = ctx.get_variable(name).attr_type;
                    if (ctx.type_exists(var_type)) {
                        auto& type = ctx.get_type(var_type);
                        if (type.has_method(method)) {
                            auto& func = type.get_method(method);
                            if (args.size() != func.params.size()) {
                                // todo handle error: argument count mismatch
                            }
                        }
                        else {
                            // todo handle error: method does not exist in this type
                        }
                    }
                }
            }
            else {
                if (ctx.function_exists(id)) {
                    auto& func = ctx.get_function(id);
                    if (args.size() != func.params.size()) {
                        // todo handle error: argument count mismatch
                    }
                }
                else {
                    // todo handle error: function does not exist
                }
            }

            for (const auto& arg : args) {
                arg->scoped_visit(ctx);
            }
        }

        void let_expr::scoped_visit(context& ctx) const {
            ctx.add_scope();

            if (!var_type.empty() && !ctx.type_exists(var_type)) {
                // todo handle error: type does not exist
            }

            value->scoped_visit(ctx);

            ctx.add_variable(id);

            body->scoped_visit(ctx);

            ctx.rollback_scope();
        }

        void assign_expr::scoped_visit(context& ctx) const {
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string attr = id.substr(id.find('.') + 1);

                if (!ctx.variable_exists(name)) {
                    // todo handle error: variable does not exist in this scope
                }
                else if (name == "self") {
                    auto var_type = ctx.get_variable(name).attr_type;
                    if (ctx.type_exists(var_type)) {
                        auto& type = ctx.get_type(var_type);
                        if (!type.has_field(attr)) {
                            // todo handle error: field does not exist in this type
                        }
                    }
                }
                else {
                    // todo handle error: cannot access field of non-self
                }
            }
            else {
                if (!ctx.variable_exists(id)) {
                    // todo handle error: variable does not exist in this scope
                }
                if (id == "self") {
                    // todo handle error: cannot assign to self
                }
            }
            value->scoped_visit(ctx);
        }

        void if_expr::scoped_visit(context& ctx) const {
            condition->scoped_visit(ctx);

            then_branch->scoped_visit(ctx);

            if (else_branch)
                else_branch->scoped_visit(ctx);
        }

        void while_expr::scoped_visit(context& ctx) const {
            condition->scoped_visit(ctx);
            body->scoped_visit(ctx);
        }

        void binary_expr::scoped_visit(context& ctx) const {
            left->scoped_visit(ctx);
            right->scoped_visit(ctx);
        }

        void unary_expr::scoped_visit(context& ctx) const {
            expr->scoped_visit(ctx);
        }

        void variable::scoped_visit(context& ctx) const {
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string attr = id.substr(id.find('.') + 1);

                if (!ctx.variable_exists(name)) {
                    // todo handle error: variable does not exist in this scope
                }
                else if (name == "self") {
                    auto var_type = ctx.get_variable(name).attr_type;
                    if (ctx.type_exists(var_type)) {
                        auto& type = ctx.get_type(var_type);
                        if (!type.has_field(attr)) {
                            // todo handle error: field does not exist in this type
                        }
                    }
                }
                else {
                    // todo handle error: cannot access field of non-self
                }
            }
            else {
                if (!ctx.variable_exists(id)) {
                    // todo handle error: variable does not exist in this scope
                }
            }
        }
    } // namespace ast
} // namespace hulk

#endif