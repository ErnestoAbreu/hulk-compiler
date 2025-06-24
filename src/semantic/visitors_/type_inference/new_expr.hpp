#ifndef HULK_SEMANTIC_TYPE_INFERENCE_NEW_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_NEW_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string new_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            auto& type = ctx.get_type(type_name.lexeme);

            while(type.params.empty())
                type = *type.parent; // Traverse up the hierarchy to find a type with parameters

            for (int i = 0; i < arguments.size(); ++i) {
                arguments[i]->infer(ctx, type.params[i].attr_type->name);
            }

            return type_name.lexeme; // Return the type of the new expression
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_NEW_EXPR_HPP