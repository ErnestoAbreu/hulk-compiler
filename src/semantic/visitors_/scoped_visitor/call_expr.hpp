#ifndef HULK_SEMANTIC_SCOPED_CALL_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_CALL_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void call_expr::scoped_visit(semantic::context& ctx) const {
            if (object.has_value()) {
                object.value()->scoped_visit(ctx);
            }
            else {
                if (!ctx.function_exists(callee.lexeme)) {
                    internal::error(callee, "Function does not exist.");
                    return;
                }

                auto& func = ctx.get_function(callee.lexeme);
                if (func.params.size() != arguments.size()) {
                    internal::error(callee, "Function expects " + std::to_string(func.params.size()) + " arguments, but got " + std::to_string(arguments.size()) + ".");
                }

                for (const auto& arg : arguments) {
                    arg->scoped_visit(ctx);
                }
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_CALL_EXPR_HPP