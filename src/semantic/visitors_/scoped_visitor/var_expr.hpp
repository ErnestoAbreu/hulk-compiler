#ifndef HULK_SEMANTIC_SCOPED_VAR_EXPR_HPP
#define HULK_SEMANTIC_SCOPED_VAR_EXPR_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void var_expr::scoped_visit(semantic::context& ctx) const {
            if (object) {
                var_expr* self = dynamic_cast<ast::var_expr*>(object->get());
                if (!self || self->object.has_value() || self->name.lexeme != "self") {
                    internal::error(name, "object in var_expr is not 'self'.");
                    return;
                }

                if (ctx.variable_exists("self")) {
                    internal::error(name, "variable 'self' doesn't have public fields.");
                    return;
                }

                if (ctx.self.empty()) {
                    internal::error(name, "variable 'self' is not defined.");
                    return;
                }

                auto& type = ctx.get_type(ctx.self);
                if (!type.has_field(name.lexeme)) {
                    internal::error(name, "field '" + name.lexeme + "' does not exist in type '" + ctx.self + "'.");
                }
            }
            else {
                if (!ctx.variable_exists(name.lexeme)) {
                    if(name.lexeme == "self" && !ctx.self.empty());
                    else 
                        internal::error(name, "variable '" + name.lexeme + "' does not exist.");
                }
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_VAR_EXPR_HPP