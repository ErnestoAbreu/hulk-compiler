#ifndef HULK_SEMANTIC_TYPE_CHECKER_DECLARATION_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_DECLARATION_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string declaration_expr::type_check(semantic::context& ctx) {
            string value_type = value->type_check(ctx);

            if (ctx.get_type(value_type) <= ctx.get_type(type.lexeme));
            else {
                internal::error(name, "type '" + type.lexeme + "' does not match value type '" + value_type + "'");
            }

            ctx.add_variable(name.lexeme, type.lexeme);

            return type.lexeme;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_DECLARATION_EXPR_HPP