#ifndef HULK_SEMANTIC_CONTEXT_BUILDER_FUNCTION_STMT_HPP
#define HULK_SEMANTIC_CONTEXT_BUILDER_FUNCTION_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void function_stmt::context_builder_visit(semantic::context& ctx) {
            if (!ctx.create_function(name.lexeme)) {
                internal::error(name, "Function already exists.");
                return;
            }

            auto& func = ctx.get_function(name.lexeme);

            if (!return_type.lexeme.empty()) {
                if (!ctx.type_exists(return_type.lexeme)) {
                    internal::error(return_type, "return type does not exist in function '" + name.lexeme + "'.");
                }
                else
                    func.return_type = std::make_shared<semantic::type>(ctx.get_type(return_type.lexeme));
            }

            for (const auto& param : parameters) {
                bool error = false;
                if (param.type.lexeme.empty()) {
                    error = !func.add_param(param.name.lexeme);
                }
                else {
                    if (!ctx.type_exists(param.type.lexeme)) {
                        internal::error(param.type, "parameter type does not exist in parameter '" + param.name.lexeme + "' in function '" + name.lexeme + "'.");
                    }
                    else
                        error = !func.add_param(param.name.lexeme, std::make_shared<semantic::type>(ctx.get_type(param.type.lexeme)));
                }

                if (error) {
                    internal::error(param.name, "parameter already exists in function '" + name.lexeme + "'.");
                }
            }
        }

    }  // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_BUILDER_FUNCTION_STMT_HPP