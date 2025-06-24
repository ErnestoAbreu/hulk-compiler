#ifndef HULK_SEMANTIC_SCOPED_NEW_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_NEW_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void new_expr::scoped_visit(semantic::context& ctx) const {
            if (!type_name.lexeme.empty() && !ctx.type_exists(type_name.lexeme)) {
                internal::error(type_name, "Type does not exist.");
            }

            auto& type = ctx.get_type(type_name.lexeme);

            while (type.params.empty()) {
                if (type.parent)
                    type = *type.parent;
                else
                    break; // No parent, exit the loop
            }

            if (type.params.size() != arguments.size()) {
                internal::error(type_name, "Type expects " + std::to_string(type.params.size()) + " arguments, but got " + std::to_string(arguments.size()) + ".");
            }

            for (const auto& arg : arguments) {
                arg->scoped_visit(ctx);
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_NEW_EXPR_HPP