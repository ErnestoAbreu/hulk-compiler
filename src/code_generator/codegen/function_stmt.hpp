#ifndef HULK_CODEGEN_FUNCTION_HPP
#define HULK_CODEGEN_FUNCTION_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Function* function_stmt::codegen() {
            // Create FunctionType
            std::vector<llvm::Type*> param_types;
            for (const auto& param : parameters) {
                llvm::Type* type = GetType(param.type.lexeme, TheModule.get());
                if(type->isStructTy())
                    param_types.push_back(type->getPointerTo());
                else 
                    param_types.push_back(type);
            }

            llvm::Type* ret_type = GetType(return_type.lexeme, TheModule.get());
            if(ret_type->isStructTy())
                ret_type = ret_type->getPointerTo();

            llvm::FunctionType* func_type = llvm::FunctionType::get(ret_type, param_types, false);

            // Create Function
            llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name.lexeme, TheModule.get());

            // Get parameters
            int idx = 0;
            for (auto& arg : func->args()) {
                arg.setName(parameters[idx].name.lexeme);
                idx++;
            }

            // Create Entry Block
            llvm::BasicBlock* EntryBB = llvm::BasicBlock::Create(*TheContext.get(), "entry", func);
            Builder->SetInsertPoint(EntryBB);

            // Allocate parameters
            NamedValues.clear();
            for (auto& arg : func->args()) {
                // Create an alloca for this variable.
                llvm::AllocaInst* alloca = CreateEntryBlockAlloca(func, arg.getType(), std::string(arg.getName()));
                // Store the initial value into the alloca.
                Builder->CreateStore(&arg, alloca);
                // Add arguments to variable symbol table.
                NamedValues[std::string(arg.getName())] = alloca;
            }

            // Get return value
            llvm::Value* ret_val = body->codegen();

            Builder->CreateRet(ret_val);

            // Validate the generated code, checking for consistency.
            llvm::verifyFunction(*func);

            return func;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_FUNCTION_HPP
