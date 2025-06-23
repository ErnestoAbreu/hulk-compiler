#ifndef HULK_SEMANTIC_SCOPED_IF_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_IF_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void if_expr::scoped_visit(semantic::context& ctx) const {
            condition->scoped_visit(ctx);

            then_branch->scoped_visit(ctx);

            for (const auto& elif : elif_branchs) {
                elif.first->scoped_visit(ctx);
                elif.second->scoped_visit(ctx);
            }

            if (else_branch) {
                else_branch.value()->scoped_visit(ctx);
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_IF_EXPR_HPP