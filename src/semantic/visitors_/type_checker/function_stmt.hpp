#ifndef HULK_SEMANTIC_TYPE_CHECKER_FUNCTION_STMT_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_FUNCTION_STMT_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void function_stmt::type_check(semantic::context& ctx) const {
            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            if (body)
                body->type_check(ctx);

            for (const auto& param : parameters)
                ctx.rollback_variable(param.name.lexeme);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_FUNCTION_STMT_HPP