#ifndef HULK_SEMANTIC_TYPE_CHECKER_FIELD_STMT_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_FIELD_STMT_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void field_stmt::type_check(semantic::context& ctx) const {
            string value_type = initializer->type_check(ctx);

            if (ctx.get_type(value_type) <= ctx.get_type(type.lexeme));
            else {
                internal::error(name,
                    "field '" + name.lexeme + "' expects type '" + type.lexeme +
                    "', but got '" + value_type + "'");
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_FIELD_STMT_HPP