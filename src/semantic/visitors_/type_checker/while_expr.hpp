#ifndef HULK_SEMANTIC_TYPE_CHECKER_WHILE_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_WHILE_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string while_expr::type_check(semantic::context& ctx) {
            string cond_type = condition->type_check(ctx);

            if (ctx.get_type(cond_type) == ctx.get_type("Boolean"));
            else {
                internal::error(0, 0, "condition is not a boolean, got '" + cond_type + "'");
            }

            return body->type_check(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_WHILE_EXPR_HPP