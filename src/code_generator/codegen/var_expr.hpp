#ifndef HULK_CODEGEN_VAR_EXPR_HPP
#define HULK_CODEGEN_VAR_EXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* var_expr::codegen() {
            llvm::AllocaInst* alloca = NamedValues[name.get_lexeme()];

            if (!alloca)
                return nullptr;

            auto* allocated_type = alloca->getAllocatedType();
            return Builder->CreateLoad(allocated_type, alloca, name.get_lexeme());
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_VAR_EXPR_HPP
