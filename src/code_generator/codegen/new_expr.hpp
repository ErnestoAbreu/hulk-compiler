#ifndef HULK_CODEGEN__EXPR_HPP
#define HULK_CODEGEN__EXPR_HPP 1

#include "../../ast/ast"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"

namespace hulk {

namespace ast {

llvm::Value* new_expr::codegen() {
  std::string struct_name = type_name.get_lexeme();
  llvm::StructType* st = llvm::StructType::getTypeByName(*TheContext, struct_name);

  auto& DL = TheModule->getDataLayout();
  uint64_t Size = DL.getTypeAllocSize(st);
  llvm::Type* IntPtrTy = llvm::Type::getInt64Ty(*TheContext);

  llvm::Function* MallocFn = TheModule->getFunction("malloc");
  if (!MallocFn) {
    llvm::FunctionType* MallocTy = llvm::FunctionType::get(Builder->getInt8Ty(), {IntPtrTy}, false);
    MallocFn = llvm::Function::Create(MallocTy, llvm::Function::ExternalLinkage, "malloc", TheModule.get());
  }

  llvm::Value* mallocSize = llvm::ConstantInt::get(IntPtrTy, Size);
  llvm::Value* rawPtr = Builder->CreateCall(MallocFn, {mallocSize}, "rawptr");

  llvm::PointerType* structPtrTy = st->getPointerTo();
  llvm::Value* objectPtr = Builder->CreateBitCast(rawPtr, structPtrTy, "objptr");

  std::string initName = "init_" + struct_name;
  if (llvm::Function* initializer = TheModule->getFunction(initName)) {
    std::vector<llvm::Value*> args;
    args.push_back(objectPtr);
    for (auto &arg: arguments) {
      llvm::Value* v = arg->codegen();
      args.push_back(v);
    }
    Builder->CreateCall(initializer, args, "initcall");
  }
  
  return objectPtr;
}

}  // namespace ast

}  // namespace hulk

#endif  // HULK_CODEGEN__EXPR_HPP
