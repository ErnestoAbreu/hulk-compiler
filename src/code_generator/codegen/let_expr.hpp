#ifndef HULK_CODEGEN_LET_EXPR_HPP
#define HULK_CODEGEN_LET_EXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* let_expr::codegen() {
            std::vector<llvm::AllocaInst*> old_bindings;

            for (const auto& decl : assignments) {
                old_bindings.push_back(NamedValues[decl->name.get_lexeme()]);
                decl->codegen();
            }

            llvm::Value* body_value = body->codegen();

            int idx = 0;
            for (const auto& decl : assignments)
                NamedValues[decl->name.get_lexeme()] = old_bindings[idx++];

            return body_value;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_LET_EXPR_HPP
