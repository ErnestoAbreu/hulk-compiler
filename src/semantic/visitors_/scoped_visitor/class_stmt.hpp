#ifndef HULK_SEMANTIC_SCOPED_CLASS_STMT_HPP
#define HULK_SEMANTIC_SCOPED_CLASS_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void class_stmt::scoped_visit(semantic::context& ctx) const {
            for (const auto& param : parameters) {
                if (!param.type.lexeme.empty() && !ctx.type_exists(param.type.lexeme)) {
                    internal::error(param.type, "Type does not exist in parameter '" + param.name.lexeme + "' in class '" + name.lexeme + "'.");
                }
                ctx.add_variable(param.name.lexeme);
            }

            if (super_class) {
                for (const auto& arg : super_class->get()->init)
                    arg->scoped_visit(ctx);
            }

            for (const auto& field : fields) {
                field->scoped_visit(ctx);
            }

            for (const auto& param : parameters)
                ctx.rollback_variable(param.name.lexeme);

            ctx.self = name.lexeme;

            for (const auto& method : methods) {
                method->scoped_visit(ctx);
            }

            ctx.self = "";
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_CLASS_STMT_HPP