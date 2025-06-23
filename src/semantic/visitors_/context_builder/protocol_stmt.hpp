#ifndef HULK_SEMANTIC_CONTEXT_BUILDER_PROTOCOL_STMT_HPP
#define HULK_SEMANTIC_CONTEXT_BUILDER_PROTOCOL_STMT_HPP 1

#include "../../../ast/ast"

namespace hulk {
    namespace ast {
        using type_ptr = std::shared_ptr<semantic::type>;

        void protocol_stmt::context_builder_visit(semantic::context& ctx) const {
            // Create the protocol in the context
            if (!ctx.create_protocol(name.lexeme)) {
                internal::error(name, "Protocol already exists.");
                return;
            }

            // Add parent protocol
            auto& protocol = ctx.get_type(name.lexeme);
            if (!super_protocol.lexeme.empty()) {
                if (!ctx.type_exists(super_protocol.lexeme) || !ctx.is_protocol(super_protocol.lexeme)) {
                    internal::error(super_protocol, "Protocol does not exist.");
                }
                else {
                    type_ptr super_proto = std::make_shared<semantic::type>(ctx.get_type(super_protocol.lexeme));
                    protocol.add_parent(super_proto);
                }
            }

            // Add methods to the protocol
            for (const auto& meth : methods) {
                semantic::method f;
                if (meth->return_type.lexeme.empty()) {
                    internal::error(meth->return_type, "return type not specified for method, in protocol '" + name.lexeme + "'.");
                }
                else {
                    if (!ctx.type_exists(meth->return_type.lexeme)) {
                        internal::error(meth->return_type, "return type does not exist, in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                    }
                    else {
                        f = semantic::method(meth->name.lexeme, std::make_shared<semantic::type>(ctx.get_type(meth->return_type.lexeme)));
                    }
                }

                for (const auto& param : meth->parameters) {
                    bool error = false;
                    if (param.type.lexeme.empty()) {
                        internal::error(param.type, "parameter type not specified, in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                    }
                    else {
                        if (!ctx.type_exists(param.type.lexeme)) {
                            internal::error(param.type, "parameter type does not exist, in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                        }
                        else {
                            error = !f.add_param(param.name.lexeme, std::make_shared<semantic::type>(ctx.get_type(param.type.lexeme)));
                        }
                    }

                    if (error) {
                        internal::error(param.name, "parameter already exists, in method '" + meth->name.lexeme + "' in protocol '" + name.lexeme + "'.");
                    }
                }

                if (!protocol.add_method(f)) {
                    internal::error(meth->name, "method already exists in protocol '" + name.lexeme + "'.");
                }
            }
        }

    }  // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_CONTEXT_BUILDER_PROTOCOL_STMT_HPP