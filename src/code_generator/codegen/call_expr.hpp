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
                if (!ArgValue) return nullptr;
                llvm::Type* ArgType = ArgValue->getType();

                llvm::Value* RetValue = ArgValue;

                const char* formatStr = nullptr;
                std::vector<llvm::Value*> PrintfArgs;

                if (ArgType->isIntegerTy(1)) {  // boolean
                    formatStr = "%s\n";
                    llvm::Value* TrueStr = Builder->CreateGlobalStringPtr("true", "true_str");
                    llvm::Value* FalseStr =
                        Builder->CreateGlobalStringPtr("false", "false_str");
                    ArgValue = Builder->CreateSelect(ArgValue, TrueStr, FalseStr);
                }
                else if (ArgType->isIntegerTy(32)) {  // integer
                    formatStr = "%d\n";
                }
                else if (ArgType->isIntegerTy(64)) {  // long integer
                    formatStr = "%ld\n";
                }
                else if (ArgType->isDoubleTy()) {  // float/double
                    formatStr = "%f\n";
                }
                else if (ArgType->isPointerTy()) {
                    formatStr = "%s\n";
                }
                else {
                    llvm::errs() << "Unsupported type in print statement\n";
                    return nullptr;
                }

                // Create format string
                llvm::Value* FormatStr = Builder->CreateGlobalStringPtr(formatStr, "fmt");
                PrintfArgs.push_back(FormatStr);
                PrintfArgs.push_back(ArgValue);

                llvm::Function* PrintfF = TheModule->getFunction("printf");
                if (!PrintfF) {
                    std::vector<llvm::Type*> printfArgs;
                    printfArgs.push_back(Builder->getInt8Ty()->getPointerTo());
                    llvm::FunctionType* printfType =
                        llvm::FunctionType::get(Builder->getInt32Ty(), printfArgs, true);
                    PrintfF =
                        llvm::Function::Create(printfType, llvm::Function::ExternalLinkage,
                            "printf", TheModule.get());
                    PrintfF->setCallingConv(llvm::CallingConv::C);
                }

                Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");

                return RetValue;
            }

            if (object) {
                llvm::Value* object_ptr = object->get()->codegen();
                if (!object_ptr || !object_ptr->getType()->isPointerTy()) {
                    internal::error("Object pointer is null or not a pointer type: " + callee.lexeme);
                    return nullptr;
                }

                string type_name = object->get()->ret_type;
                llvm::Value* func_ptr = getMethodPtr(object_ptr, type_name, callee.lexeme);

                llvm::FunctionType* func_type = MethodTypes[type_name][callee.lexeme];

                func_ptr = Builder->CreateBitCast(func_ptr, func_type->getPointerTo(), "casted_method_ptr");

                std::vector<llvm::Value*> args_values;
                args_values.push_back(object_ptr);

                for (auto i = 0; i < arguments.size(); i++) {
                    args_values.push_back(arguments[i]->codegen());
                    if (!args_values.back()) return nullptr;
                }

                llvm::Value* return_value = Builder->CreateCall(func_type, func_ptr, args_values, "calltmp");

                return return_value;
            }
            else {
                llvm::Function* calleeF = TheModule->getFunction(callee.lexeme);

                if (!calleeF) {
                    internal::error("Function not found: " + callee.lexeme, "CODEGEN");
                    return nullptr;
                }

                std::vector<llvm::Value*> args_values;
                for (auto i = 0; i < arguments.size(); i++) {
                    args_values.push_back(arguments[i]->codegen());
                    if (!args_values.back()) return nullptr;
                }

                return Builder->CreateCall(calleeF, args_values, "calltmp");
            }
        }

    }  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_CALLEXPR_HPP