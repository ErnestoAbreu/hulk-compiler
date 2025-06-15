#ifndef HULK_CODEGEN_PROGRAM_HPP
#define HULK_CODEGEN_PROGRAM_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* program::codegen() const {
            // Generate code for each statement in the program
            for (const auto& stmt : statements)
                if (!stmt->codegen())
                    return nullptr;

            // Crear la función main
            std::vector<llvm::Type*> main_args;
            llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ast::TheContext), main_args, false);

            llvm::Function* main_func = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", ast::TheModule.get());

            // Crear un bloque básico para la función main
            llvm::BasicBlock* entry = llvm::BasicBlock::Create(*ast::TheContext, "entry", main_func);
            ast::Builder->SetInsertPoint(entry);

            // Generate code for the main expression
            if (main) {
                auto* value = main->codegen();
                // Retornar 0 al final de la función main
                ast::Builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ast::TheContext), 0));

                return value;
            }

            return nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_PROGRAM_HPP
