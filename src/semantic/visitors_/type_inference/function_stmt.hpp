#ifndef HULK_SEMANTIC_TYPE_INFERENCE_FUNCTION_STMT_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_FUNCTION_STMT_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string function_stmt::infer(semantic::context& ctx, const string& shouldbe_type) {
            auto& func = ctx.get_function(name.lexeme);

            for (const auto& param : parameters) {
                ctx.add_variable(param.name.lexeme, param.type.lexeme);
            }

            string inferred_type = body->infer(ctx, return_type.lexeme);

            // Infer return type if not specified
            if (return_type.lexeme.empty()) {
                if (inferred_type.empty()) {
                    semantic::add_infer_error(name.line, name.column, "return type not inferred");
                }
                else {
                    return_type.lexeme = inferred_type;
                    func.return_type = std::make_shared<semantic::type>(ctx.get_type(return_type.lexeme));
                    semantic::repeat_infer = true; // If return type was inferred, we need to repeat inference
                }
            }

            for (auto& param : parameters) {
                // Infer parameter type if not specified
                if (param.type.lexeme.empty()) {
                    inferred_type = ctx.get_variable_type(param.name.lexeme);
                    if (inferred_type.empty()) {
                        semantic::add_infer_error(param.name.line, param.name.column, "parameter type not inferred");
                    }
                    else {
                        param.type.lexeme = inferred_type;
                        func.get_param(param.name.lexeme).attr_type = std::make_shared<semantic::type>(ctx.get_type(param.type.lexeme));
                        semantic::repeat_infer = true; // If parameter type was inferred, we need to repeat inference
                    }
                }

                ctx.rollback_variable(param.name.lexeme);
            }

            return return_type.lexeme;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_FUNCTION_STMT_HPP