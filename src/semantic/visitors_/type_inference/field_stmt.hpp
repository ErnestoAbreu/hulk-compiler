#ifndef HULK_SEMANTIC_TYPE_INFERENCE_FIELD_STMT_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_FIELD_STMT_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string field_stmt::infer(semantic::context& ctx, const string& shouldbe_type) {
            string inferred_type = initializer->infer(ctx, type.lexeme);

            if (type.lexeme.empty() && !inferred_type.empty()) {
                // Infer field type
                type.lexeme = inferred_type;
                semantic::repeat_infer = true; // If type was inferred, we need to repeat inference
            }

            if (type.lexeme.empty())
                semantic::add_infer_error(name.line, name.column, "field type not inferred");

            return type.lexeme;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_FIELD_STMT_HPP