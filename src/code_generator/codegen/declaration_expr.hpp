#ifndef HULK_CODEGEN_DECLARATION_EXPR_HPP
#define HULK_CODEGEN_DECLARATION_EXPR_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Value* declaration_expr::codegen() {
            llvm::Value* init_val = value->codegen();
            if (!init_val) return nullptr;

            llvm::Function* function = Builder->GetInsertBlock()->getParent();

            llvm::AllocaInst* alloca = CreateEntryBlockAlloca(function, init_val->getType(), name.get_lexeme());

            Builder->CreateStore(init_val, alloca);

            NamedValues[name.get_lexeme()] = alloca;

            return init_val;
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_DECLARATION_EXPR_HPP