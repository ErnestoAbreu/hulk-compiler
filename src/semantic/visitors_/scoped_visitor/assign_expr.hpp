#ifndef HULK_SEMANTIC_SCOPED_ASSIGN_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_ASSIGN_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void assign_expr::scoped_visit(semantic::context& ctx) const {
            variable->scoped_visit(ctx);

            value->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_ASSIGN_EXPR_HPP