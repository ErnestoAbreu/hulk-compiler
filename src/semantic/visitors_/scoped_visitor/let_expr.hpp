#ifndef HULK_SEMANTIC_SCOPED_LET_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_LET_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void let_expr::scoped_visit(semantic::context& ctx) const {
            for (const auto& assignment : assignments)
                assignment->scoped_visit(ctx);

            body->scoped_visit(ctx);

            for (const auto& assignment : assignments)
                ctx.rollback_variable(assignment->name.lexeme);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_LET_EXPR_HPP