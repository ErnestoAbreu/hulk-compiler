#ifndef HULK_SEMANTIC_TYPE_CHECKER_LET_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_LET_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string let_expr::type_check(semantic::context& ctx) {
            for (const auto& assignment : assignments)
                assignment->type_check(ctx);

            string body_type = body->type_check(ctx);

            for (const auto& assignment : assignments)
                ctx.rollback_variable(assignment->name.lexeme);

            return body_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_LET_EXPR_HPP