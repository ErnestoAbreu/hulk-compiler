#ifndef HULK_SEMANTIC_TYPE_INFERENCE_UNARY_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_UNARY_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string unary_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            if (ast::unary_op::MINUS == op) {
                expression->infer(ctx, "Number");
                return "Number";
            }

            if (ast::unary_op::NOT == op) {
                expression->infer(ctx, "Boolean");
                return "Boolean";
            }

            return shouldbe_type; // If unary operator is not recognized, return the expected type
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_UNARY_EXPR_HPP