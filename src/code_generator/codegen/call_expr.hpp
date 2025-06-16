#ifndef HULK_CODEGEN_CALLEXPR_HPP
#define HULK_CODEGEN_CALLEXPR_HPP 1

#include "../../ast/ast"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

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

    const char* formatStr = nullptr;
    std::vector<llvm::Value*> PrintfArgs;

    if (ArgType->isIntegerTy(1)) {  // boolean
      formatStr = "%s\n";
      llvm::Value* TrueStr = Builder->CreateGlobalStringPtr("true", "true_str");
      llvm::Value* FalseStr =
          Builder->CreateGlobalStringPtr("false", "false_str");
      ArgValue = Builder->CreateSelect(ArgValue, TrueStr, FalseStr);
    } else if (ArgType->isIntegerTy(32)) {  // integer
      formatStr = "%d\n";
    } else if (ArgType->isIntegerTy(64)) {  // long integer
      formatStr = "%ld\n";
    } else if (ArgType->isDoubleTy()) {  // float/double
      formatStr = "%f\n";
    } else if (ArgType->isPointerTy()) {
      formatStr = "%s\n";
    } else {
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

    return Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");
  }

  llvm::Function* CalleF = TheModule->getFunction(callee.lexeme);

  std::vector<llvm::Value*> argsValues;
  for (auto i = 0; i < arguments.size(); i++) {
    argsValues.push_back(arguments[i]->codegen());
    if (!argsValues.back()) return nullptr;
  } 

  return Builder->CreateCall(CalleF, argsValues, "calltmp");
}

}  // namespace ast
}  // namespace hulk

#endif  // HULK_CODEGEN_CALLEXPR_HPP