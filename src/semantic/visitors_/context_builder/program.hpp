#ifndef HULK_SEMANTIC_CONTEXT_BUILDER_PROGRAM_HPP
#define HULK_SEMANTIC_CONTEXT_BUILDER_PROGRAM_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void add_builtin_types(semantic::context& ctx) {
            // Add built-in protocols to the context
            ctx.create_protocol("Iterable");

            auto& iterable_type = ctx.get_type("Iterable");

            semantic::method current("current", make_shared<semantic::type>(ctx.get_type("Object")));
            semantic::method next("next", make_shared<semantic::type>(ctx.get_type("Boolean")));
            iterable_type.add_method(current);
            iterable_type.add_method(next);

            ctx.create_protocol("Enumerable");

            auto& enumerable_type = ctx.get_type("Enumerable");

            semantic::method iter("iter", make_shared<semantic::type>(ctx.get_type("Iterable")));
            enumerable_type.add_method(iter);


            // Add built-in types to the context
            ctx.create_type("Object");

            ctx.create_type("String");
            auto& string_type = ctx.get_type("String");
            string_type.add_parent(make_shared<semantic::type>(ctx.get_type("Object")));

            ctx.create_type("Number");
            auto& number_type = ctx.get_type("Number");
            number_type.add_parent(make_shared<semantic::type>(ctx.get_type("Object")));

            ctx.create_type("Boolean");
            auto& boolean_type = ctx.get_type("Boolean");
            boolean_type.add_parent(make_shared<semantic::type>(ctx.get_type("Object")));
        }

        void add_builtin_functions(semantic::context& ctx) {
            // Add built-in functions to the context
            ctx.create_function("print");
            auto& print_func = ctx.get_function("print");
            print_func.add_param("value", make_shared<semantic::type>(ctx.get_type("Object")));
            print_func.return_type = make_shared<semantic::type>(ctx.get_type("Object"));

            ctx.create_function("sqrt");
            auto& sqrt_func = ctx.get_function("sqrt");
            sqrt_func.add_param("value", make_shared<semantic::type>(ctx.get_type("Number")));
            sqrt_func.return_type = make_shared<semantic::type>(ctx.get_type("Number"));

            ctx.create_function("sin");
            auto& sin_func = ctx.get_function("sin");
            sin_func.add_param("value", make_shared<semantic::type>(ctx.get_type("Number")));
            sin_func.return_type = make_shared<semantic::type>(ctx.get_type("Number"));

            ctx.create_function("cos");
            auto& cos_func = ctx.get_function("cos");
            cos_func.add_param("value", make_shared<semantic::type>(ctx.get_type("Number")));
            cos_func.return_type = make_shared<semantic::type>(ctx.get_type("Number"));

            ctx.create_function("exp");
            auto& exp_func = ctx.get_function("exp");
            exp_func.add_param("value", make_shared<semantic::type>(ctx.get_type("Number")));
            exp_func.return_type = make_shared<semantic::type>(ctx.get_type("Number"));

            ctx.create_function("log");
            auto& log_func = ctx.get_function("log");
            log_func.add_param("base", make_shared<semantic::type>(ctx.get_type("Number")));
            log_func.add_param("value", make_shared<semantic::type>(ctx.get_type("Number")));
            log_func.return_type = make_shared<semantic::type>(ctx.get_type("Number"));

            ctx.create_function("rand");
            auto& rand_func = ctx.get_function("rand");
            rand_func.return_type = make_shared<semantic::type>(ctx.get_type("Number"));
        }

        void program::context_builder_visit(semantic::context& ctx) const {
            add_builtin_types(ctx);
            add_builtin_functions(ctx);

            for (const auto& stmt : statements)
                stmt->context_builder_visit(ctx);
        }

    }  // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_BUILDER_PROGRAM_HPP