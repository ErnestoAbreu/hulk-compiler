#ifndef HULK_SEMANTIC_TYPE_INFERENCE_CLASS_STMT_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_CLASS_STMT_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string class_stmt::infer(semantic::context& ctx, const string& shouldbe_type) {
            auto& this_type = ctx.get_type(name.lexeme);

            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            if (super_class.has_value()) {
                auto& parent_type = ctx.get_type(super_class.value()->name.lexeme);

                auto& parent_args = super_class.value()->init;
                for (int i = 0; i < parent_args.size(); ++i) {
                    parent_args[i]->infer(ctx, parent_type.params[i].attr_type->name);
                }
            }

            int idx = 0;
            for (const auto& field : fields) {
                string inferred_type = field->infer(ctx);

                // Add inferred type to context
                if (!this_type.fields[idx].attr_type && inferred_type != "") {
                    this_type.fields[idx].attr_type = semantic::shared_type[ctx.get_type(inferred_type)];
                }
                idx++;
            }

            idx = 0;
            for (auto& param : parameters) {
                if (param.type.lexeme.empty()) {
                    // Infer param type
                    param.type.lexeme = ctx.get_variable_type(param.name.lexeme);

                    if (param.type.lexeme != "") {
                        this_type.params[idx].attr_type = semantic::shared_type[ctx.get_type(param.type.lexeme)];
                        semantic::repeat_infer = true;
                    }
                    else {
                        semantic::add_infer_error(param.name.line, param.name.column, "param type not inferred");
                    }

                }
                idx++;
                ctx.rollback_variable(param.name.lexeme);
            }

            ctx.self = name.lexeme;

            for (auto& meth : methods) {
                semantic::method& func = this_type.get_method(meth->name.lexeme);

                for (const auto& param : meth->parameters) {
                    ctx.add_variable(param.name.lexeme, param.type.lexeme);
                }

                string inferred_type = meth->body->infer(ctx, meth->return_type.lexeme);

                // Infer return type if not specified
                if (meth->return_type.lexeme.empty()) {
                    if (inferred_type.empty()) {
                        semantic::add_infer_error(name.line, name.column, "return type not inferred");
                    }
                    else {
                        meth->return_type.lexeme = inferred_type;
                        func.return_type = semantic::shared_type[ctx.get_type(meth->return_type.lexeme)];
                        semantic::repeat_infer = true; // If return type was inferred, we need to repeat inference
                    }
                }

                for (auto& param : meth->parameters) {
                    // Infer parameter type if not specified
                    if (param.type.lexeme.empty()) {
                        inferred_type = ctx.get_variable_type(param.name.lexeme);
                        if (inferred_type.empty()) {
                            semantic::add_infer_error(param.name.line, param.name.column, "parameter type not inferred");
                        }
                        else {
                            param.type.lexeme = inferred_type;
                            func.get_param(param.name.lexeme).attr_type = semantic::shared_type[ctx.get_type(param.type.lexeme)];
                            semantic::repeat_infer = true; // If parameter type was inferred, we need to repeat inference
                        }
                    }

                    ctx.rollback_variable(param.name.lexeme);
                }
            }

            ctx.self = "";

            return name.lexeme; // Ignore return type for type definitions
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_CLASS_STMT_HPP