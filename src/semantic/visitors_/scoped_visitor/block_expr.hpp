#ifndef HULK_SEMANTIC_SCOPED_BLOCK_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_BLOCK_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void block_expr::scoped_visit(semantic::context& ctx) const {
            for (const auto& expr : expressions)
                expr->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_BLOCK_EXPR_HPP