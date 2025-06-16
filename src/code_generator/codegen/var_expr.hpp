#ifndef HULK_CODEGEN_VAR_EXPR_HPP
#define HULK_CODEGEN_VAR_EXPR_HPP 1

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"  
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* var_expr::codegen() {
            if (object) {
                llvm::Value* object_value = (*object)->codegen();

                llvm::PointerType* object_type = llvm::cast<llvm::PointerType>(object_value->getType());
                llvm::StructType* st = llvm::cast<llvm::StructType>(object_type->getNonOpaquePointerElementType());
                
                std::string struct_name = st->getName().str();
                auto struct_it = StructFieldIndices.find(struct_name);

                auto field_map = struct_it->second;
                auto field_it = field_map.find(name.get_lexeme());

                auto index = field_it->second;
                llvm::Value* gep = Builder->CreateStructGEP(st, object_value, index, "fieldptr");
                return Builder->CreateLoad(st->getElementType(index), gep, name.get_lexeme());
            }

            auto it = NamedValues.find(name.get_lexeme());
            llvm::Value* alloca = it->second;
            llvm::Type* type = alloca->getType()->getNonOpaquePointerElementType();
            return Builder->CreateLoad(type, alloca, name.get_lexeme());
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_VAR_EXPR_HPP
