#ifndef HULK_CODEGEN_VAR_EXPR_HPP
#define HULK_CODEGEN_VAR_EXPR_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Value* var_expr::codegen() {
            if (object) {
                llvm::Type* object_type = GetType((*object)->ret_type, TheModule.get());

                llvm::Value* object_ptr = (*object)->codegen();

                if (!object_ptr || !object_ptr->getType()->isPointerTy()) {
                    internal::error("Object pointer is null or not a pointer type: " + name.get_lexeme());
                    return nullptr;
                }

                unsigned int index = GetStructFieldIndex(object_type->getStructName().str(), name.get_lexeme());
                llvm::Value* member_ptr = Builder->CreateStructGEP(object_type, object_ptr, index, "memberptr");

                return Builder->CreateLoad(object_type->getStructElementType(index), member_ptr, name.get_lexeme());
            }
            else {
                llvm::AllocaInst* alloca = NamedValues[name.get_lexeme()];

                if (!alloca) {
                    internal::error("Variable allocation not found: " + name.get_lexeme());
                    return nullptr;
                }

                // llvm::Type* allocated_type = GetType(ret_type, TheModule.get());

                // if (allocated_type->isStructTy())
                //     allocated_type = allocated_type->getPointerTo();

                return Builder->CreateLoad(alloca->getAllocatedType(), alloca, name.get_lexeme());
            }
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_VAR_EXPR_HPP
