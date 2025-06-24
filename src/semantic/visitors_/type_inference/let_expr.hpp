#ifndef HULK_SEMANTIC_TYPE_INFERENCE_LET_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_LET_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string let_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            for (const auto& assignment : assignments)
                assignment->infer(ctx);

            string inferred_type = body->infer(ctx, shouldbe_type);

            for(const auto& assignment : assignments)
                ctx.rollback_variable(assignment->name.lexeme);

            return inferred_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_LET_EXPR_HPP