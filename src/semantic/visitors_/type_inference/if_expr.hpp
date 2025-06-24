#ifndef HULK_SEMANTIC_TYPE_INFERENCE_IF_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_IF_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string if_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            condition->infer(ctx, "Boolean");

            string then_type = then_branch->infer(ctx, shouldbe_type);

            for (const auto& elif : elif_branchs) {
                elif.first->infer(ctx, "Boolean");

                string elif_type = elif.second->infer(ctx, shouldbe_type);

                if (then_type.empty() || elif_type.empty());
                else
                    then_type = ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(elif_type))->name;
            }

            if (else_branch.has_value()) {
                string else_type = else_branch.value()->infer(ctx, shouldbe_type);

                if (then_type.empty() || else_type.empty());
                else
                    then_type = ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(else_type))->name;
            }

            if (then_type.empty())
                then_type = shouldbe_type;

            return then_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_IF_EXPR_HPP