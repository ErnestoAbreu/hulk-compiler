#ifndef HULK_SEMANTIC_SCOPED_PROGRAM_HPP
#define HULK_SEMANTIC_SCOPED_PROGRAM_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void program::scoped_visit(semantic::context& ctx) const {
            for (const auto& stmt : statements)
                stmt->scoped_visit(ctx);

            if (main)
                main->scoped_visit(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_PROGRAM_HPP