#ifndef HULK_SEMANTIC_SCOPED_WHILE_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_WHILE_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void while_expr::scoped_visit(semantic::context& ctx) const {
            condition->scoped_visit(ctx);
            body->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_WHILE_EXPR_HPP