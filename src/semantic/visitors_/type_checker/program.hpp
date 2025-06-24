#ifndef HULK_SEMANTIC_TYPE_CHECKER_PROGRAM_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_PROGRAM_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void program::type_check(semantic::context& ctx) const {
            for (const auto& stmt : statements)
                stmt->type_check(ctx);

            if (main)
                main->type_check(ctx);
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_PROGRAM_HPP