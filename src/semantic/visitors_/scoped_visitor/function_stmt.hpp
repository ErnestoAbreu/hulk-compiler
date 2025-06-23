#ifndef HULK_SEMANTIC_SCOPED_FUNCTION_STMT_HPP
#define HULK_SEMANTIC_SCOPED_FUNCTION_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void function_stmt::scoped_visit(semantic::context& ctx) const {
            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme);

            if (body)
                body->scoped_visit(ctx);

            for (const auto& param : parameters)
                ctx.rollback_variable(param.name.lexeme);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_FUNCTION_STMT_HPP