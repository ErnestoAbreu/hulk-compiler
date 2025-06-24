#ifndef HULK_SEMANTIC_TYPE_INFERENCE_VAR_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_VAR_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string var_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            if (object.has_value()) {
                string inferred_type = object.value()->infer(ctx);

                auto& obj_type = ctx.get_type(inferred_type);

                auto& field = obj_type.get_field(name.lexeme);

                if (!field.attr_type)
                    return shouldbe_type; // If field type is not set, return the expected type

                return field.attr_type->name; // Return the type of the field
            }
            else {
                if(name.lexeme == "self" && !ctx.variable_exists(name.lexeme)) 
                    return ctx.self;
                
                auto& var_type = ctx.get_variable_type(name.lexeme);

                if (var_type.empty()) {
                    var_type = shouldbe_type; // If variable type is not set, use the expected type
                }

                return var_type;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_VAR_EXPR_HPP