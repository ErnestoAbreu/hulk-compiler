#ifndef HULK_SEMANTIC_TYPE_CHECKER_PROTOCOL_STMT_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_PROTOCOL_STMT_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        void protocol_stmt::type_check(semantic::context& ctx) const {
            auto& protocol = ctx.get_type(name.lexeme);
            auto& parent = *protocol.parent;

            for (const auto& method : protocol.methods) {
                if (!parent.has_method(method.name))
                    continue;

                auto& parent_method = parent.get_method(method.name);
                if (method.return_type <= parent_method.return_type);
                else {
                    internal::error(name, "can't extends protocol, return types are not covariant.");
                }

                if (method.params.size() != parent_method.params.size()) {
                    internal::error(name, "can't extends protocol, " + method.name + " arguments mismatch");
                    continue;
                }

                for (size_t i = 0; i < method.params.size(); ++i)
                    if (parent_method.params[i].attr_type <= method.params[i].attr_type);
                    else {
                        internal::error(name, "can't extends protocol, " + method.name + "argument " + std::to_string(i) + " are not contravariant.");
                    }
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_PROTOCOL_STMT_HPP