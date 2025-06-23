#ifndef HULK_SEMANTIC_SCOPED_FIELD_STMT_HPP
#define HULK_SEMANTIC_SCOPED_FIELD_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void field_stmt::scoped_visit(semantic::context& ctx) const {
            initializer->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_FIELD_STMT_HPP