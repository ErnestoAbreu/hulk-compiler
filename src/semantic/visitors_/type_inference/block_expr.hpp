#ifndef HULK_SEMANTIC_TYPE_INFERENCE_BLOCK_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_BLOCK_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string block_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            for (int i = 0; i < expressions.size() - 1; ++i)
                expressions[i]->infer(ctx);

            return expressions.back()->infer(ctx, shouldbe_type);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_BLOCK_EXPR_HPP