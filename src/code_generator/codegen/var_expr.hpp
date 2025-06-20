#ifndef HULK_CODEGEN_VAR_EXPR_HPP
#define HULK_CODEGEN_VAR_EXPR_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Value* var_expr::codegen() {
            if (object.has_value()) {
                llvm::Value* object_value = (*object)->codegen();

                if (!object_value) {
                    llvm::errs() << "Error in var_expr codegen: object value is null: " << name.get_lexeme() << "\n";
                    internal::error_found = true;
                    return nullptr;
                }

                llvm::Type* object_type = object_value->getType();
                if (object_type->isPointerTy()) {
                    llvm::errs() << "Error in var_expr object is a pointer: " << *object_type << "\n";
                    internal::error_found = true;
                    return nullptr;
                }

                llvm::AllocaInst* object_alloca = Builder->CreateAlloca(object_type, nullptr, "object_alloca");
                Builder->CreateStore(object_value, object_alloca);

                unsigned int index = GetStructFieldIndex(object_type->getStructName().str(), name.get_lexeme());
                llvm::Value* member_ptr = Builder->CreateStructGEP(object_type, object_alloca, index, "memberptr");

                return Builder->CreateLoad(object_type->getStructElementType(index), member_ptr, name.get_lexeme());
            }
            else {
                llvm::AllocaInst* alloca = NamedValues[name.get_lexeme()];

                if (!alloca) {
                    llvm::errs() << "Error in var_expr codegen: " << name.get_lexeme() << "\n";
                    internal::error_found = true;
                    return nullptr;
                }

                auto* allocated_type = alloca->getAllocatedType();
                return Builder->CreateLoad(allocated_type, alloca, name.get_lexeme());
            }
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_VAR_EXPR_HPP
