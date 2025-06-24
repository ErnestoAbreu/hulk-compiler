#ifndef HULK_SEMANTIC_TYPE_CHECKER_FOR_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_FOR_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string for_expr::type_check(semantic::context& ctx) {
            string iterable_type = iterable->type_check(ctx);
            auto& iter_type = ctx.get_type(iterable_type);

            if (iter_type <= ctx.get_type("Iterable")) {
                auto& current_method = iter_type.get_method("current");

                if (*current_method.return_type <= ctx.get_type(var_type.lexeme));
                else {
                    internal::error(var_name,
                        "type '" + var_type.lexeme + "' does not match iterable's current type '" + current_method.return_type->name + "'");
                }
            }
            else if (iter_type <= ctx.get_type("Enumerable")) {
                auto& iter_method = iter_type.get_method("iter");

                iter_type = *iter_method.return_type;
                auto& current_method = iter_type.get_method("current");

                if (*current_method.return_type <= ctx.get_type(var_type.lexeme));
                else {
                    internal::error(var_name,
                        "type '" + var_type.lexeme + "' does not match iterable's current type '" + current_method.return_type->name + "'");
                }
            }
            else {
                internal::error(var_name,
                    "iterable is not an Iterable or Enumerable, got '" + iterable_type + "'");
            }

            ctx.add_variable(var_name.lexeme, var_type.lexeme);

            string body_type = body->type_check(ctx);

            ctx.rollback_variable(var_name.lexeme);

            return body_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_FOR_EXPR_HPP        

