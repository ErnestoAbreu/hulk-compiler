#ifndef HULK_SEMANTIC_TYPE_INFERENCE_WHILE_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_WHILE_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string while_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            condition->infer(ctx, "Boolean");
            return body->infer(ctx, shouldbe_type);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_WHILE_EXPR_HPP