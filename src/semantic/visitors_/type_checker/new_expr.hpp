#ifndef HULK_SEMANTIC_TYPE_CHECKER_NEW_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_NEW_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string new_expr::type_check(semantic::context& ctx) {
            int args_count = arguments.size();
            auto& current = ctx.get_type(type_name.lexeme);

            for (int i = 0; i < args_count; ++i) {
                string arg_type = arguments[i]->type_check(ctx);
                if (ctx.get_type(arg_type) <= *current.params[i].attr_type);
                else {
                    internal::error(type_name,
                        "type '" + type_name.lexeme + "' expects argument " + std::to_string(i + 1) +
                        " of type '" + current.params[i].attr_type->name + "', but got '" + arg_type + "'");
                }
            }

            return type_name.lexeme;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_NEW_EXPR_HPP