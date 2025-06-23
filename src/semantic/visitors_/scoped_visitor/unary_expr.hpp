#ifndef HULK_SEMANTIC_SCOPED_UNARY_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_UNARY_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void unary_expr::scoped_visit(semantic::context& ctx) const {
            expression->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_UNARY_EXPR_HPP