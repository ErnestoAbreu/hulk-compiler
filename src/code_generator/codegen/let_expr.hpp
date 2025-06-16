#ifndef HULK_CODEGEN_LET_EXPR_HPP
#define HULK_CODEGEN_LET_EXPR_HPP 1

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"  
#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* let_expr::codegen() {
            for (const auto &decl: assignments)
                decl->codegen();
            
            llvm::Value* body_value = body->codegen();
            
            return body_value;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_LET_EXPR_HPP
