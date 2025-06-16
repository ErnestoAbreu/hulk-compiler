#ifndef HULK_CODEGEN_DECLARATION_EXPR_HPP
#define HULK_CODEGEN_DECLARATION_EXPR_HPP 1

#include <iostream>

#include "../../ast/ast"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

namespace hulk {

namespace ast {

llvm::Value* declaration_expr::codegen() {
  llvm::Value* initVal = value->codegen();
  if (!initVal) return nullptr;

  llvm::Type* llvmType = initVal->getType();
  llvm::Function* function = Builder->GetInsertBlock()->getParent();
  llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
  llvm::AllocaInst* alloca = tmpB.CreateAlloca(llvmType, nullptr, name.get_lexeme());

  Builder->CreateStore(initVal, alloca);

  NamedValues[name.get_lexeme()] = alloca;

  return initVal;
}

}  // namespace ast

}  // namespace hulk

#endif  // HULK_CODEGEN_DECLARATION_EXPR_HPP