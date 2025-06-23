#ifndef HULK_SEMANTIC_SCOPED_DECLARATION_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_DECLARATION_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void declaration_expr::scoped_visit(semantic::context& ctx) const {
            if (!type.lexeme.empty() && !ctx.type_exists(type.lexeme)) {
                internal::error(type, "Type does not exist in declaration '" + name.lexeme + "'.");
            }

            if (value) {
                value->scoped_visit(ctx);
            }

            ctx.add_variable(name.lexeme, type.lexeme);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_DECLARATION_EXPR_HPP