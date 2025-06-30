#ifndef HULK_SEMANTIC_SCOPED_CLASS_STMT_HPP
#define HULK_SEMANTIC_SCOPED_CLASS_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void class_stmt::scoped_visit(semantic::context& ctx) const {
            for (const auto& param : parameters) {
                if (!param.type.lexeme.empty() && !ctx.type_exists(param.type.lexeme)) {
                    internal::error(param.type, "Type does not exist in parameter '" + param.name.lexeme + "' in class '" + name.lexeme + "'.");
                }
                ctx.add_variable(param.name.lexeme);
            }

            if (super_class) {
                int init_size = super_class->get()->init.size();
                auto& parent = ctx.get_type(super_class->get()->name.lexeme);

                while(parent.params.empty()) {
                    if (parent.parent)
                        parent = *parent.parent;
                    else
                        break; // No parent, exit the loop
                }

                if (init_size != parent.params.size()) {
                    internal::error(super_class->get()->name, "Type '" + super_class->get()->name.lexeme +
                        "' expects " + std::to_string(parent.params.size()) + " arguments, but got " + std::to_string(init_size) + ".");
                }

                for (const auto& arg : super_class->get()->init)
                    arg->scoped_visit(ctx);
            }

            for (const auto& field : fields) {
                field->scoped_visit(ctx);
            }

            for (const auto& param : parameters)
                ctx.rollback_variable(param.name.lexeme);

            ctx.self = name.lexeme;

            for (const auto& method : methods) {
                ctx.current_method = nullptr;
                if(ctx.get_type(name.lexeme).parent 
                && ctx.get_type(name.lexeme).parent->has_method(method->name.lexeme)) {
                    ctx.current_method = &(ctx.get_type(name.lexeme).parent->get_method(method->name.lexeme));
                }
                
                method->scoped_visit(ctx);
            }

            ctx.self = "";
            ctx.current_method = nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_SCOPED_CLASS_STMT_HPP