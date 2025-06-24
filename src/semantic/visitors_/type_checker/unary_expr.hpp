#ifndef HULK_SEMANTIC_TYPE_CHECKER_UNARY_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_UNARY_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string unary_expr::type_check(semantic::context& ctx) {
            string expr_type = expression->type_check(ctx);

            if (op == unary_op::MINUS) {
                if (expr_type != "Number")
                    internal::error(token, token.lexeme + " operand is not a number");

                return "Number";
            }

            if (op == unary_op::NOT) {
                if (expr_type != "Boolean")
                    internal::error(token, token.lexeme + " operand is not a boolean");

                return "Boolean";
            }

            // unknown operator
            internal::error(token, "unknown operator: " + token.lexeme);
            return "Object";
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_UNARY_EXPR_HPP