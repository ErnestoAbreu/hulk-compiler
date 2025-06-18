#ifndef HULK_CODEGEN_FUNCTION_HPP
#define HULK_CODEGEN_FUNCTION_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Function* function_stmt::codegen() {
            // Create FunctionType
            std::vector<llvm::Type*> ParamTypes;
            for (const auto& param : parameters)
                ParamTypes.push_back(code_generator::get_type(param.type.lexeme, TheModule.get()));

            llvm::FunctionType* FuncType = llvm::FunctionType::get(code_generator::get_type(return_type.lexeme, TheModule.get()), ParamTypes, false);

            // Create Function
            llvm::Function* Func = llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, name.lexeme, TheModule.get());

            // Get parameters
            int idx = 0;
            for (auto& Arg : Func->args()) {
                Arg.setName(parameters[idx].name.lexeme);
                idx++;
            }

            // Create Entry Block
            llvm::BasicBlock* EntryBB = llvm::BasicBlock::Create(*TheContext.get(), "entry", Func);
            Builder->SetInsertPoint(EntryBB);

            // Allocate parameters
            NamedValues.clear();
            for (auto& Arg : Func->args()) {
                // Create an alloca for this variable.
                llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(Func, std::string(Arg.getName()), Arg.getType());

                // Store the initial value into the alloca.
                Builder->CreateStore(&Arg, Alloca);

                // Add arguments to variable symbol table.
                NamedValues[std::string(Arg.getName())] = Alloca;
            }

            // Get return value
            llvm::Value* RetVal = body->codegen();

            Builder->CreateRet(RetVal);

            // Validate the generated code, checking for consistency.
            llvm::verifyFunction(*Func);

            return Func;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_FUNCTION_HPP
