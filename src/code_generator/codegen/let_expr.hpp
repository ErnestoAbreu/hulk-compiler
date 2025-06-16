#ifndef HULK_CODEGEN_LET_EXPR_HPP
#define HULK_CODEGEN_LET_EXPR_HPP 1

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"  
#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* let_expr::codegen() {
            auto old = NamedValues;

            llvm::BasicBlock* curBB = Builder->GetInsertBlock();
            llvm::Function* TheFunction = curBB->getParent();

            for (const auto &decl: assignments) {
                std::string name = decl->name.get_lexeme();
                llvm::Value* init = decl->value->codegen();

                llvm::AllocaInst* alloca = CreateEntryBlockAlloca(TheFunction, name, init->getType());
                Builder->CreateStore(init, alloca);

                NamedValues[name] = alloca;
            }
            
            llvm::Value* body_value = body->codegen();
            
            NamedValues = std::move(old);
            return body_value;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_LET_EXPR_HPP
