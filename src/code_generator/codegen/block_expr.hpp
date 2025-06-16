#ifndef HULK_CODEGEN_BLOCK_EXPR_HPP
#define HULK_CODEGEN_BLOCK_EXPR_HPP 1

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"  
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

#include "../../ast/ast"

namespace hulk {
    namespace ast {

      llvm::Value* block_expr::codegen() {
        llvm::Value* last_value = nullptr;
        for (const auto &expr: expressions) {
          last_value = expr->codegen();
        }
        return last_value;
      }
      
    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_BLOCK_EXPR_HPP
