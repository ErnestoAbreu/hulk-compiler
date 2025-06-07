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

        void def_type::scoped_visit(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : params)
                ctx.add_variable(param.id);

            for (const auto& field : fields) {
                field->scoped_visit(ctx);
            }

            ctx.rollback_scope();

            ctx.add_scope();

            auto& type = ctx.get_type(id);
            ctx.add_variable("self", type.name);

            for (const auto& method : methods) {
                method->scoped_visit(ctx);
            }

            ctx.rollback_scope();
        }

        void def_function::scoped_visit(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : params)
                ctx.add_variable(param.id);

            if (body)
                body->scoped_visit(ctx);

            ctx.rollback_scope();
        }

        void def_field::scoped_visit(context& ctx) const {
            if (value)
                value->scoped_visit(ctx);
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

                if (!ctx.variable_exists(name)) {
                    // todo handle error: variable does not exist in this scope
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

            value->scoped_visit(ctx);

            ctx.add_variable(id);

            body->scoped_visit(ctx);

            ctx.rollback_scope();
        }

        void assign_expr::scoped_visit(context& ctx) const {
            if (!ctx.variable_exists(id)) {
                // todo handle error: variable does not exist in this scope
            }
            if (id == "self") {
                // todo handle error: cannot assign to self
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

                if (!ctx.variable_exists(name)) {
                    // todo handle error: variable does not exist in this scope
                }
            }
            else {
                if (!ctx.variable_exists(id)) {
                    // todo handle error: variable does not exist in this scope
                }
            }
        }
    }
}

#endif