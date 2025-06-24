#ifndef HULK_SEMANTIC_TYPE_CHECKER_IF_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_IF_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string if_expr::type_check(semantic::context& ctx) {
            string cond_type = condition->type_check(ctx);

            if (ctx.get_type(cond_type) == ctx.get_type("Boolean"));
            else {
                internal::error(0, 0, "condition is not a boolean, got '" + cond_type + "'");
            }

            string then_type = then_branch->type_check(ctx);

            for (const auto& elif : elif_branchs) {
                string elif_cond_type = elif.first->type_check(ctx);
                if (ctx.get_type(elif_cond_type) == ctx.get_type("Boolean"));
                else {
                    internal::error(0, 0, "elif condition is not a boolean, got '" + elif_cond_type + "'");
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

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_IF_EXPR_HPP