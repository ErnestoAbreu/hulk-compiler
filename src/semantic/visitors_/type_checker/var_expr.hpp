#ifndef HULK_SEMANTIC_TYPE_CHECKER_VAR_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_VAR_EXPR_HPP

#include "../../../ast/ast"
namespace hulk {
    namespace ast {

        string var_expr::type_check(semantic::context& ctx) {
            if (object.has_value()) {
                string _type = object.value()->type_check(ctx);
                auto& obj_type = ctx.get_type(_type);

                auto& field = obj_type.get_field(name.lexeme);
                return field.attr_type->name;
            }
            else {
                if (name.lexeme == "self" && !ctx.variable_exists(name.lexeme))
                    return ctx.self;

                auto& var_type = ctx.get_variable_type(name.lexeme);
                return var_type;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_VAR_EXPR_HPP