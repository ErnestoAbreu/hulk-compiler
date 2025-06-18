#ifndef HULK_CODEGEN_VAR_EXPR_HPP
#define HULK_CODEGEN_VAR_EXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* var_expr::codegen() {
            if (object.has_value()) {
                auto* object_value = (*object)->codegen();
                if (!object_value) {
                    llvm::errs() << "Error in var_expr codegen: object value is null: " << name.get_lexeme() << "\n";
                    return nullptr;
                }

                auto* object_type = object_value->getType();
                if (!object_type->isPointerTy()) {
                    llvm::errs() << "Error in var_expr codegen: expected pointer type, got " << *object_type << "\n";
                    return nullptr;
                }

                auto* object_ptr = Builder->CreateLoad(object_type->getPointerTo(), object_value, "object_ptr");

                if (NamedValues.find(name.get_lexeme()) == NamedValues.end()) {
                    llvm::errs() << "Error in var_expr codegen: variable '" << name.get_lexeme() << "' not found\n";
                    return nullptr;
                }

                //todo
                return nullptr;
            }
            else {
                llvm::AllocaInst* alloca = NamedValues[name.get_lexeme()];

                if (!alloca) {
                    llvm::errs() << "Error in var_expr codegen: " << name.get_lexeme() << "\n";
                    return nullptr;
                }

                auto* allocated_type = alloca->getAllocatedType();
                return Builder->CreateLoad(allocated_type, alloca, name.get_lexeme());
            }
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_VAR_EXPR_HPP
