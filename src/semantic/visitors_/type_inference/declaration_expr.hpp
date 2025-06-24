#ifndef HULK_SEMANTIC_TYPE_INFERENCE_DECLARATION_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_DECLARATION_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string declaration_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            if (value) {
                string inferred_type = value->infer(ctx, type.lexeme);

                if (type.lexeme.empty()) {
                    if (inferred_type.empty()) {
                        semantic::add_infer_error(name.line, name.column, "variable type not inferred, is necessary to annotate it.");
                    }
                    else {
                        type.lexeme = inferred_type;
                        semantic::repeat_infer = true;
                    }
                }
            }

            ctx.add_variable(name.lexeme, type.lexeme);

            return type.lexeme;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_DECLARATION_EXPR_HPP