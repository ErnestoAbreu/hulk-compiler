#ifndef HULK_SEMANTIC_TYPE_CHECKER_ASSIGN_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_ASSIGN_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string assign_expr::type_check(semantic::context& ctx) {
            string var_type = variable->type_check(ctx);

            string value_type = value->type_check(ctx);

            if (ctx.get_type(value_type) <= ctx.get_type(var_type));
            else {
                internal::error(type, "type '" + var_type + "' does not match value type '" + value_type + "'");
            }

            return var_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_ASSIGN_EXPR_HPP