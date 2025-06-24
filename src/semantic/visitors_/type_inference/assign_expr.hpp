#ifndef HULK_SEMANTIC_TYPE_INFERENCE_ASSIGN_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_ASSIGN_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string assign_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            string inferred_type = variable->infer(ctx, shouldbe_type);
            return value->infer(ctx, inferred_type);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_ASSIGN_EXPR_HPP