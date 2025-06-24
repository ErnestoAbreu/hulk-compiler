#ifndef HULK_SEMANTIC_TYPE_INFERENCE_BINARY_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_BINARY_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string binary_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            if (semantic::number_ops.count(op)) {
                left->infer(ctx, "Number");
                right->infer(ctx, "Number");
                return "Number";
            }

            if (semantic::boolean_ops.count(op)) {
                left->infer(ctx, "Boolean");
                right->infer(ctx, "Boolean");
                return "Boolean";
            }

            if (semantic::comparison_ops.count(op)) {
                left->infer(ctx, "Number");
                right->infer(ctx, "Number");
                return "Boolean";
            }

            if (semantic::equality_ops.count(op)) {
                left->infer(ctx);
                right->infer(ctx);
                return "Boolean";
            }

            if (semantic::string_ops.count(op)) {
                left->infer(ctx);
                right->infer(ctx);
                return "String";
            }

            return shouldbe_type;
        }
        
    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_BINARY_EXPR_HPP