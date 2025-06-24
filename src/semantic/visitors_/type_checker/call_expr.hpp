#ifndef HULK_SEMANTIC_TYPE_CHECKER_CALL_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_CALL_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string call_expr::type_check(semantic::context& ctx) {
            if (object.has_value()) {
                string obj_type = object.value()->type_check(ctx);
                auto& obj = ctx.get_type(obj_type);

                auto& method = obj.get_method(callee.lexeme);

                for (size_t i = 0; i < arguments.size(); ++i) {
                    string arg_type = arguments[i]->type_check(ctx);
                    if (ctx.get_type(arg_type) <= *method.params[i].attr_type);
                    else {
                        internal::error(callee,
                            "Method '" + callee.lexeme + "' expects argument " + std::to_string(i + 1) +
                            " of type '" + method.params[i].attr_type->name + "', but got '" + arg_type + "'");
                    }
                }

                return method.return_type->name;
            }
            else {
                auto& func = ctx.get_function(callee.lexeme);

                for (size_t i = 0; i < arguments.size(); ++i) {
                    string arg_type = arguments[i]->type_check(ctx);
                    if (ctx.get_type(arg_type) <= *func.params[i].attr_type);
                    else {
                        internal::error(callee,
                            "Function '" + callee.lexeme + "' expects argument " + std::to_string(i + 1) +
                            " of type '" + func.params[i].attr_type->name + "', but got '" + arg_type + "'");
                    }
                }

                return func.return_type->name;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_CALL_EXPR_HPP