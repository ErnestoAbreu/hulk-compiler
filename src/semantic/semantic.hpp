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
            if (internal::error_found) {
                return -1; // Error in context building
            }

            std::cerr << "Context built successfully." << std::endl;

            program.scoped_visit(ctx);
            if (internal::error_found) {
                return -1; // Error in scoped visit
            }

            std::cerr << "Scoped visit completed successfully." << std::endl;

            program.infer(ctx);
            if (internal::error_found) {
                return -1; // Error in type inference
            }

            std::cerr << "Type inference completed successfully." << std::endl;

            program.type_check(ctx);
            if (internal::error_found) {
                return -1; // Error in type checking
            }

            std::cerr << "Type checking completed successfully." << std::endl;

            return 0;
        }

    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_HPP