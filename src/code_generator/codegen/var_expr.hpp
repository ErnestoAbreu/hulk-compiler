#ifndef HULK_CODEGEN_VAR_EXPR_HPP
#define HULK_CODEGEN_VAR_EXPR_HPP 1

#include "../../ast/ast"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

namespace hulk {
    
namespace ast {

llvm::Value* var_expr::codegen() {
  auto it = NamedValues.find(name.get_lexeme());
  if (it == NamedValues.end()) return nullptr;

  llvm::Value* alloca = it->second;

  llvm::AllocaInst* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(alloca);
  if (!allocaInst) return nullptr;

  llvm::Type* valueType = allocaInst->getAllocatedType();
  return Builder->CreateLoad(valueType, alloca, name.get_lexeme());
}

}  // namespace ast

}  // namespace hulk

#endif  // HULK_CODEGEN_VAR_EXPR_HPP
