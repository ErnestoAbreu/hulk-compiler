#ifndef HULK_SEMANTIC_HPP
#define HULK_SEMANTIC_HPP 1

#include "context.hpp"
#include "visitors"
#include "../ast/ast"

namespace hulk {
    namespace semantic {

        int analyze(const ast::program& program) {
            context ctx;

            // Initialize the context with built-in types and protocols

            ctx.create_protocol("Iterable");
            ctx.create_protocol("Enumerable");

            ctx.create_type("Object");
            ctx.create_type("Number");
            ctx.create_type("Boolean");
            ctx.create_type("String");

            // Add built-in functions

            ctx.create_function("print");
            ctx.get_function("print").add_param("value", "Object");
            ctx.get_function("print").return_type = "Object";

            // Start the context building process

            program.context_builder_visit(ctx);
            if(internal::error_found) {
                return -1; // Error in context building
            }

            // std::cerr << "Context built successfully." << std::endl;

            program.scoped_visit(ctx);
            if(internal::error_found) {
                return -1; // Error in scoped visit
            }

            // std::cerr << "Scoped visit completed successfully." << std::endl;

            program.infer(ctx);
            if(internal::error_found) {
                return -1; // Error in type inference
            }

            // std::cerr << "Type inference completed successfully." << std::endl;

            program.type_check(ctx);
            if(internal::error_found) {
                return -1; // Error in type checking
            }

            // std::cerr << "Type checking completed successfully." << std::endl;
    
            return 0;
        }

    } // namespace semantic
} // namespace hulk

#endif // HULK_SEMANTIC_HPP