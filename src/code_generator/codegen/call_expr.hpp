#ifndef HULK_CODEGEN_NAME_HPP
#define HULK_CODEGEN_NAME_HPP 1

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

                // Create format string
                llvm::Value* FormatStr = Builder->CreateGlobalStringPtr("%f\n", "fmt", 0, TheModule.get());

                // Call printf
                std::vector<llvm::Value*> PrintfArgs;
                PrintfArgs.push_back(FormatStr);
                PrintfArgs.push_back(ArgValue);

                llvm::Function* PrintfF = TheModule->getFunction("printf");
                return Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");
            }

            // Function* CalleeF = TheModule->getFunction(Callee);
            // if (!CalleeF)
            //     return LogErrorV("Unknown function referenced");

            // if (CalleeF->arg_size() != Args.size())
            //     return LogErrorV("Incorrect # arguments passed");

            // std::vector<Value*> ArgsV;
            // for (unsigned i = 0, e = Args.size(); i != e; ++i) {
            //     ArgsV.push_back(Args[i]->codegen());
            //     if (!ArgsV.back())
            //         return nullptr;
            // }

            // return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
            return nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_NAME_HPP
