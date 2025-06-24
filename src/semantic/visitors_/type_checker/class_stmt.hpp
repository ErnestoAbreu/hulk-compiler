#ifndef HULK_SEMANTIC_TYPE_CHECKER_CLASS_STMT_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_CLASS_STMT_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void class_stmt::type_check(semantic::context& ctx) const {
            auto& this_type = ctx.get_type(name.lexeme);

            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            if (super_class.has_value()) {
                auto& parent_type = ctx.get_type(super_class.value()->name.lexeme);
                auto& parent_args = super_class.value()->init;

                for (int i = 0; i < parent_args.size(); ++i) {
                    string arg_type = parent_args[i]->type_check(ctx);
                    if (ctx.get_type(arg_type) <= *parent_type.params[i].attr_type);
                    else {
                        internal::error(super_class.value()->name,
                            "type '" + name.lexeme + "' expects argument " + std::to_string(i + 1) +
                            " of type '" + parent_type.params[i].attr_type->name + "', but got '" + arg_type + "'");
                    }
                }
            }

            for (const auto& field : fields) {
                field->type_check(ctx);
            }

            for (const auto& param : parameters)
                ctx.rollback_variable(param.name.lexeme);

            ctx.self = name.lexeme;

            for (const auto& method : methods)
                method->type_check(ctx);

            ctx.self = "";
        }
        
    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_CLASS_STMT_HPP