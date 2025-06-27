#ifndef HULK_SEMANTIC_TYPE_INFERENCE_CALL_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_CALL_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string call_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            if (object.has_value()) {
                string object_type = object.value()->infer(ctx);

                if (object_type.empty()) {
                    return shouldbe_type; // If object type is not inferred, return expected type
                }

                auto& type = ctx.get_type(object_type);

                if (!type.has_method(callee.lexeme)) {
                    semantic::add_infer_error(callee.line, callee.column, "Method '" + callee.lexeme + "' does not exist in type '" + object_type + "'.");
                    return shouldbe_type; // If method does not exist, return expected type
                }

                auto& method = type.get_method(callee.lexeme);
                if (method.params.size() != arguments.size()) {
                    semantic::add_infer_error(callee.line, callee.column,
                        "Method '" + callee.lexeme + "' expects " + std::to_string(method.params.size()) +
                        " arguments, but got " + std::to_string(arguments.size()) + ".");
                    return shouldbe_type; // If argument count does not match, return expected type
                }

                for (size_t i = 0; i < arguments.size(); ++i) {
                    arguments[i]->infer(ctx, method.params[i].attr_type->name);
                }

                if (method.return_type)
                    return method.return_type->name;

                return shouldbe_type; // If method return type is not defined, return expected type
            }
            else {
                if(callee.lexeme == "print") {
                    return arguments[0]->infer(ctx,"Object");
                }

                if(callee.lexeme == "base") {
                    for (const auto& arg : arguments) {
                        arg->infer(ctx);
                    }
                    return "String";
                }

                auto& func = ctx.get_function(callee.lexeme);

                for (size_t i = 0; i < arguments.size(); ++i) {
                    if(func.params[i].attr_type)
                        arguments[i]->infer(ctx, func.params[i].attr_type->name);
                    else 
                         arguments[i]->infer(ctx);
                }

                if (func.return_type)
                    return func.return_type->name;

                return shouldbe_type; // If function return type is not defined, return expected type
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_CALL_EXPR_HPP