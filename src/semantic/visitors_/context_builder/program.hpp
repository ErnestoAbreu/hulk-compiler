#ifndef HULK_SEMANTIC_CONTEXT_BUILDER_PROGRAM_HPP
#define HULK_SEMANTIC_CONTEXT_BUILDER_PROGRAM_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void program::context_builder_visit(semantic::context& ctx) const {
            for (const auto& stmt : statements)
                stmt->context_builder_visit(ctx);
        }

    }  // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_BUILDER_PROGRAM_HPP