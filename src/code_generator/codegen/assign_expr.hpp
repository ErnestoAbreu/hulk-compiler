#ifndef HULK_CODEGEN_ASSIGN_EXPR_HPP
#define HULK_CODEGEN_ASSIGN_EXPR_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Value* assign_expr::codegen() {
            llvm::Value* value_obj = value->codegen();
            if (!value_obj) {
                llvm::errs() << "Error: Failed to generate code for value in assignment.\n";
                internal::error_found = true;
                return nullptr;
            }

            var_expr* var = dynamic_cast<var_expr*>(variable.get());
            if (!var) {
                llvm::errs() << "Error: Variable in assignment is not a var_expr.\n";
                internal::error_found = true;
                return nullptr;
            }

            llvm::Value* alloca = GetAllocation(var);
            if (!alloca) {
                llvm::errs() << "Error: Failed to get allocation for variable in assignment.\n";
                internal::error_found = true;
                return nullptr;
            }

            Builder->CreateStore(value_obj, alloca);

            return value_obj;
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_ASSIGN_EXPR_HPP