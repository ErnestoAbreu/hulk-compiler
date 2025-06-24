#ifndef HULK_SEMANTIC_TYPE_INFERENCE_FOR_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_FOR_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string for_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            string inferred_iter_type = iterable->infer(ctx, "Iterable");

            if (!inferred_iter_type.empty()) {
                auto& iter_type = ctx.get_type(inferred_iter_type);

                if (iter_type.has_method("current")) {
                    auto& current_method = iter_type.get_method("current");

                    if (var_type.lexeme.empty()) {
                        var_type.lexeme = current_method.return_type->name;
                        semantic::repeat_infer = true; // Repeat inference if type was inferred
                    }
                }
            }

            if (var_type.lexeme.empty()) {
                semantic::add_infer_error(var_name.line, var_name.column, "variable type was not inferred, is necessary to annotate it.");
            }

            ctx.add_variable(var_name.lexeme, var_type.lexeme);

            string body_type = body->infer(ctx, shouldbe_type);

            ctx.rollback_variable(var_name.lexeme);

            if (body_type.empty()) {
                body_type = shouldbe_type; // If body type is not inferred, return the expected type
            }

            return body_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_FOR_EXPR_HPP