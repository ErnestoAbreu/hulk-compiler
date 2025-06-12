#ifndef HULK_SEMANTIC_HPP
#define HULK_SEMANTIC_HPP 1

#include "context.hpp"
#include "visitors"
#include "../ast/ast"

namespace hulk {
    namespace semantic {

        int analyze(const ast::program& program) {
            context ctx;


            program.context_builder_visit(ctx);
    
            return 0;
        }

    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_HPP