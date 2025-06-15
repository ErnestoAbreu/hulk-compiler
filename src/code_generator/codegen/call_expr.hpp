#ifndef HULK_CODEGEN_CALLEXPR_HPP
#define HULK_CODEGEN_CALLEXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* call_expr::codegen() {
            // Special case for print
            if (callee.lexeme == "print") {
                if (arguments.size() != 1) {
                    llvm::errs() << "print function expects exactly one argument\n";
                    return nullptr;
                }

                llvm::Value* ArgValue = arguments[0]->codegen();
                if (!ArgValue)
                    return nullptr;

                // Create format double
                llvm::Value* FormatDouble = Builder->CreateGlobalStringPtr("%f\n", "fmt", 0, TheModule.get());

                // Create format string
                llvm::Value* FormatString = Builder->CreateGlobalStringPtr("%s\n", "fmt", 0, TheModule.get());

                // Call printf
                if(ArgValue->getType()->isDoubleTy()) {
                    // Number
                    std::vector<llvm::Value*> PrintfArgs;
                    PrintfArgs.push_back(FormatDouble);
                    PrintfArgs.push_back(ArgValue);

                    llvm::Function* PrintfF = TheModule->getFunction("printf");
                    return Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");
                }
                else if(ArgValue->getType()->isIntegerTy()) {
                    // Boolean
                    std::vector<llvm::Value*> PrintfArgs;
                    PrintfArgs.push_back(FormatString);
                    // if 1 then true, else false
                    llvm::Value* BoolValue = Builder->CreateICmpNE(ArgValue, llvm::ConstantInt::get(ArgValue->getType(), 0), "boolcmp");
                    PrintfArgs.push_back(BoolValue);
                    llvm::Function* PrintfF = TheModule->getFunction("printf");
                    return Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");
                }
                else {
                    // String
                    std::vector<llvm::Value*> PrintfArgs;
                    PrintfArgs.push_back(FormatString);
                    PrintfArgs.push_back(ArgValue);
                    llvm::Function* PrintfF = TheModule->getFunction("printf");
                    return Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");
                }
            }

            llvm::Function* CalleeF = TheModule->getFunction(callee.lexeme);

            std::vector<llvm::Value*> ArgsV;
            for (unsigned i = 0, e = arguments.size(); i != e; ++i) {
                ArgsV.push_back(arguments[i]->codegen());
                if (!ArgsV.back())
                    return nullptr;
            }

            return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_CALLEXPR_HPP
