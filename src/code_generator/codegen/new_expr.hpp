#ifndef HULK_CODEGEN__EXPR_HPP
#define HULK_CODEGEN__EXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* new_expr::codegen() {
            std::string struct_name = "type." + type_name.get_lexeme();

            llvm::StructType* struct_type = llvm::StructType::getTypeByName(*TheContext, struct_name);
            if (!struct_type) {
                llvm::errs() << "Error: no struct type" << struct_name << "\n";
                internal::error_found = true;
                return nullptr;
            }

            llvm::Function* type_ctor = TheModule->getFunction(struct_name + "._ctor");
            if (!type_ctor) {
                llvm::errs() << "Error: no constructor for type" << struct_name << "\n";
                internal::error_found = true;
                return nullptr;
            }

            std::vector<llvm::Value*> args_values;
            for (const auto& arg : arguments) {
                llvm::Value* arg_value = arg->codegen();
                if (!arg_value) {
                    llvm::errs() << "Error: Failed to codegen argument for new expression\n";
                    internal::error_found = true;
                    return nullptr;
                }
                args_values.push_back(arg_value);
            }

            llvm::Value* object_ptr = Builder->CreateCall(type_ctor, args_values, struct_name + "_ptr");

            llvm::Value* object = Builder->CreateLoad(struct_type, object_ptr, struct_name + "obj");

            return object;
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN__EXPR_HPP
