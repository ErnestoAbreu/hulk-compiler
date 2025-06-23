#ifndef HULK_SEMANTIC_SCOPED_FOR_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_FOR_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void for_expr::scoped_visit(semantic::context& ctx) const {
            if (!var_type.lexeme.empty() && !ctx.type_exists(var_type.lexeme)) {
                internal::error(var_type, "Type does not exist in for loop variable '" + var_name.lexeme + "'.");
            }

            iterable->scoped_visit(ctx);

            ctx.add_variable(var_name.lexeme);

            if (body) {
                body->scoped_visit(ctx);
            }

            ctx.rollback_variable(var_name.lexeme);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_FOR_EXPR_HPP