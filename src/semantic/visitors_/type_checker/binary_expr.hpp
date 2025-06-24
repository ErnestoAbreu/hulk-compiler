#ifndef HULK_SEMANTIC_TYPE_CHECKER_BINARY_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_BINARY_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string binary_expr::type_check(semantic::context& ctx) {
            string left_type = left->type_check(ctx);
            string right_type = right->type_check(ctx);

            if (semantic::number_ops.count(op)) {
                if (left_type != "Number")
                    internal::error(token, token.lexeme + " left operand is not a number");
                if (right_type != "Number")
                    internal::error(token, token.lexeme + " right operand is not a number");

                return "Number";
            }

            if (semantic::boolean_ops.count(op)) {
                if (left_type != "Boolean")
                    internal::error(token, token.lexeme + " left operand is not a boolean");
                if (right_type != "Boolean")
                    internal::error(token, token.lexeme + " right operand is not a boolean");

                return "Boolean";
            }

            if (semantic::comparison_ops.count(op)) {
                if (left_type != "Number")
                    internal::error(token, token.lexeme + " left operand is not a number");
                if (right_type != "Number")
                    internal::error(token, token.lexeme + " right operand is not a number");

                return "Boolean";
            }

            if (semantic::equality_ops.count(op)) {
                return "Boolean";
            }

            if (semantic::string_ops.count(op)) {
                if (left_type != "String" && left_type != "Number")
                    internal::error(token, token.lexeme + " left operand is not a string");
                if (right_type != "String" && right_type != "Number")
                    internal::error(token, token.lexeme + " right operand is not a string");

                return "String";
            }

            // unknown operator
            internal::error(token, "unknown operator: " + token.lexeme);
            return "Object";
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_BINARY_EXPR_HPP