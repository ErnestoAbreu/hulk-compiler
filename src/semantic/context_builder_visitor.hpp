#ifndef CONTEXT_BUILDER_VISITOR_HPP
#define CONTEXT_BUILDER_VISITOR_HPP 1

#include "context.hpp"
#include "../ast/ast"

namespace hulk {
    namespace ast {
        using type_ptr = semantic::type_ptr;
        using protocol_ptr = semantic::protocol_ptr;
        using context = semantic::context;
        using method = semantic::method;

        void program::context_builder_visit(context& ctx) const {
            for (const auto& stmt : statements) {
                stmt->context_builder_visit(ctx);
            }
        }

        void protocol_stmt::context_builder_visit(context& ctx) const {
            // Create the protocol in the context
            if (!ctx.create_protocol(name.lexeme)) {
                internal::semantic_error(name.line, name.column, "Protocol '" + name.lexeme + "' already exists.");
                return;
            }

            auto& protocol = ctx.get_protocol(name.lexeme);
            if (!super_protocol.lexeme.empty()) {
                if (!ctx.protocol_exists(super_protocol.lexeme)) {
                    internal::semantic_error(super_protocol.line, super_protocol.column, "Protocol '" + super_protocol.lexeme + "' does not exist.");
                }
                else {
                    protocol_ptr super_proto = std::make_shared<semantic::protocol>(ctx.get_protocol(super_protocol.lexeme));
                    protocol.add_parent(super_proto);
                }
            }

            // Add methods to the protocol
            for (const auto& meth : methods) {
                method f;
                if (meth->return_type.lexeme.empty()) {
                    internal::semantic_error(meth->return_type.line, meth->return_type.column,
                        "return type not specified for method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                }
                else {
                    if (!ctx.type_exists(meth->return_type.lexeme)) {
                        internal::semantic_error(meth->return_type.line, meth->return_type.column,
                            "return type '" + meth->return_type.lexeme + "' does not exist for method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                    }
                    else {
                        f = method(meth->name.lexeme, meth->return_type.lexeme);
                    }
                }

                for (const auto& param : meth->parameters) {
                    bool error = false;
                    if (param.type.lexeme.empty()) {
                        internal::semantic_error(param.type.line, param.type.column,
                            "parameter type not specified for parameter '" + param.name.lexeme + "' in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                    }
                    else {
                        if (!ctx.type_exists(param.type.lexeme)) {
                            internal::semantic_error(param.type.line, param.type.column,
                                "parameter type '" + param.type.lexeme + "' does not exist for parameter '" + param.name.lexeme + "' in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                        }
                        else {
                            error = f.add_param(param.name.lexeme, param.type.lexeme);
                        }
                    }

                    if (error) {
                        internal::semantic_error(param.name.line, param.name.column,
                            "parameter '" + param.name.lexeme + "' already exists in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                    }
                }

                if (!protocol.add_method(f)) {
                    internal::semantic_error(meth->name.line, meth->name.column,
                        "method '" + meth->name.lexeme + "' already exists in protocol '" + name.lexeme + "'.");
                }
            }
        }

        void class_stmt::context_builder_visit(context& ctx) const {
            // Create the type in the context
            if (!ctx.create_type(name.lexeme)) {
                // todo: handle error, type already exists
                return;
            }

            // Get the type from the context
            auto& type = ctx.get_type(name.lexeme);

            for (const auto& param : parameters) {
                bool error = false;
                if (param.type.lexeme.empty()) {
                    error = type.add_param(param.name.lexeme);
                }
                else {
                    if (!ctx.type_exists(param.type.lexeme)) {
                        // todo: handle error, type does not exist
                    }
                    else
                        error = type.add_param(param.name.lexeme, param.type.lexeme);
                }
                if (error) {
                    // todo: handle error, param already exists
                }
            }

            if (super_class.has_value()) {
                if (!ctx.type_exists(super_class->get()->name.lexeme)) {
                    //todo: handle error, parent type does not exist
                }
                else {
                    type_ptr super_type = std::make_shared<semantic::type>(ctx.get_type(super_class->get()->name.lexeme));
                    type.add_parent(super_type);
                }
            }

            // Add fields to the type
            for (const auto& field : fields) {
                bool error = false;
                if (field->type.lexeme.empty()) {
                    error = type.add_field(field->name.lexeme);
                }
                else {
                    if (!ctx.type_exists(field->type.lexeme)) {
                        //todo: handle error, type does not exist
                    }
                    else
                        error = type.add_field(field->name.lexeme, field->type.lexeme);
                }
                if (error) {
                    //todo: handle error, field already exists
                }
            }

            // Add methods to the type
            for (const auto& meth : methods) {
                method f;
                if (meth->return_type.lexeme.empty()) {
                    f = method(meth->name.lexeme);
                }
                else {
                    if (!ctx.type_exists(meth->return_type.lexeme)) {
                        //todo: handle error, return type does not exist
                    }
                    else
                        f = method(meth->name.lexeme, meth->return_type.lexeme);
                }

                for (const auto& param : meth->parameters) {
                    bool error = false;
                    if (param.type.lexeme.empty()) {
                        error = f.add_param(param.name.lexeme);
                    }
                    else {
                        if (!ctx.type_exists(param.type.lexeme)) {
                            //todo: handle error, parameter type does not exist
                        }
                        else
                            error = f.add_param(param.name.lexeme, param.type.lexeme);
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

        void function_stmt::context_builder_visit(context& ctx) const {
            if (!ctx.create_function(name.lexeme)) {
                // todo: handle error, function already exists
                return;
            }

            auto& func = ctx.get_function(name.lexeme);

            if (!return_type.lexeme.empty()) {
                if (!ctx.type_exists(return_type.lexeme)) {
                    //todo: handle error, return type does not exist
                }
                else
                    func.return_type = return_type.lexeme;
            }
            else {
                func.return_type = ""; // No return type specified
            }

            for (const auto& param : parameters) {
                bool error = false;
                if (param.type.lexeme.empty()) {
                    error = func.add_param(param.name.lexeme);
                }
                else {
                    if (!ctx.type_exists(param.type.lexeme)) {
                        // todo: handle error, parameter type does not exist
                    }
                    else
                        error = func.add_param(param.name.lexeme, param.type.lexeme);
                }

                if (error) {
                    // todo: handle error, parameter already exists
                }
            }
        }

    } // namespace ast
} // namespace hulk

#endif // CONTEXT_BUILDER_VISITOR_HPP