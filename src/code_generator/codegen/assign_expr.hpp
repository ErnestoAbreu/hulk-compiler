#ifndef HULK_CODEGEN_ASSIGN_EXPR_HPP
#define HULK_CODEGEN_ASSIGN_EXPR_HPP 1

#include <iostream>

#include "../../ast/ast"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

namespace hulk {

namespace ast {

llvm::Value* assign_expr::codegen() {
  llvm::Value* rhs = value->codegen();
  if (!rhs) return nullptr;

  auto* var = dynamic_cast<var_expr*>(variable.get());
  if (!var) return nullptr;

  if (var->object) {
    llvm::Value* object_value = (*var->object)->codegen();
    if (!object_value) return nullptr;

    llvm::Type* objType = object_value->getType();
    if (!objType->isPointerTy()) return nullptr;

    auto* ptrType = llvm::cast<llvm::PointerType>(objType);
    llvm::Type* pointedType = ptrType->getContainedType(0);

    llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(pointedType);
    if (!st) return nullptr;

    std::string struct_name = st->getName().str();
    auto struct_it = StructFieldIndices.find(struct_name);
    if (struct_it == StructFieldIndices.end()) return nullptr;

    const auto& field_map = struct_it->second;
    auto field_it = field_map.find(var->name.get_lexeme());
    if (field_it == field_map.end()) return nullptr;

    unsigned index = field_it->second;
    llvm::Value* gep = Builder->CreateStructGEP(st, object_value, index, "fieldptr");
    Builder->CreateStore(rhs, gep);
  } else {
    auto it = NamedValues.find(var->name.get_lexeme());
    // std::cerr << "hrere\n";
    if (it == NamedValues.end()) return nullptr;
    llvm::Value* alloca = it->second;
    Builder->CreateStore(rhs, alloca);
  }

  return rhs;
}


}  // namespace ast

}  // namespace hulk

#endif  // HULK_CODEGEN_ASSIGN_EXPR_HPP