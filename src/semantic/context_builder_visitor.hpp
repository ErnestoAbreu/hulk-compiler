#ifndef CONTEXT_BUILDER_VISITOR_HPP
#define CONTEXT_BUILDER_VISITOR_HPP

#include "context.hpp"
#include "../ast/node.hpp"

namespace hulk {
    namespace ast {
        using context = semantic::context;
        using method = semantic::method;

        void program::context_builder_visit(context& ctx) const {
            for (const auto& stmt : statements) {
                stmt->context_builder_visit(ctx);
            }
        }

        void def_type::context_builder_visit(context& ctx) const {
            // Create the type in the context
            if (!ctx.create_type(id)) {
                // todo: handle error, type already exists
                return;
            }

            // Get the type from the context
            auto& type = ctx.get_type(id);

            for (const auto& param : params) {
                bool error = false;
                if (param.type.empty()) {
                    error = type.add_param(param.id);
                }
                else {
                    if (!ctx.type_exists(param.type)) {
                        // todo: handle error, type does not exist
                    }
                    else
                        error = type.add_param(param.id, param.type);
                }
                if (error) {
                    // todo: handle error, param already exists
                }
            }


            if (!ctx.type_exists(parent)) {
                //todo: handle error, parent type does not exist
            }
            else {
                type.add_parent(ctx.get_type(parent));
            }

            // Add fields to the type
            for (const auto& field : fields) {
                bool error = false;
                if (field->var_type.empty()) {
                    error = type.add_field(field->id);
                }
                else {
                    if (!ctx.type_exists(field->var_type)) {
                        //todo: handle error, type does not exist
                    }
                    else
                        error = type.add_field(field->id, field->var_type);
                }
                if (error) {
                    //todo: handle error, field already exists
                }
            }

            // Add methods to the type
            for (const auto& meth : methods) {
                method f;
                if (meth->return_type.empty()) {
                    f = method(meth->id);
                }
                else {
                    if (!ctx.type_exists(meth->return_type)) {
                        //todo: handle error, return type does not exist
                    }
                    else
                        f = method(meth->id, meth->return_type);
                }

                for (const auto& param : meth->params) {
                    bool error = false;
                    if (param.type.empty()) {
                        error = f.add_param(param.id);
                    }
                    else {
                        if (!ctx.type_exists(param.type)) {
                            //todo: handle error, parameter type does not exist
                        }
                        else
                            error = f.add_param(param.id, param.type);
                    }
                    if (error) {
                        //todo: handle error, parameter already exists
                    }
                }

                if (!type.add_method(f)) {
                    //todo: handle error, method already exists
                }
            }
        }

        void def_function::context_builder_visit(context& ctx) const {
            if (!ctx.create_function(id)) {
                // todo: handle error, function already exists
                return;
            }

            auto& func = ctx.get_function(id);

            if (!return_type.empty()) {
                if (!ctx.type_exists(return_type)) {
                    //todo: handle error, return type does not exist
                    return;
                }
                func.return_type = return_type;
            }
            else {
                func.return_type = ""; // No return type specified
            }

            for (const auto& param : params) {
                bool error = false;
                if (param.type.empty()) {
                    error = func.add_param(param.id);
                }
                else {
                    if (!ctx.type_exists(param.type)) {
                        // todo: handle error, parameter type does not exist
                    }
                    error = func.add_param(param.id, param.type);
                }

                if (error) {
                    // todo: handle error, parameter already exists
                }
            }
        }

    } // namespace ast
} // namespace hulk

#endif // CONTEXT_BUILDER_VISITOR_HPP