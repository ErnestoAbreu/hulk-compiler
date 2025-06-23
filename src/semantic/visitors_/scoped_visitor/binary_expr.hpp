#ifndef HULK_SEMANTIC_SCOPED_BINARY_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_BINARY_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void binary_expr::scoped_visit(semantic::context& ctx) const {
            left->scoped_visit(ctx);
            right->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_BINARY_EXPR_HPP