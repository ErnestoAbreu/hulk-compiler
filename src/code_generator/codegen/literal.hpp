#ifndef HULK_CODEGEN_LITERAL_HPP
#define HULK_CODEGEN_LITERAL_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* literal_expr::codegen() {
            if (holds_alternative<nullptr_t>(value))
                return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(*TheContext), 0));

            if (holds_alternative<string>(value))
                return Builder->CreateGlobalStringPtr(std::get<std::string>(value), "strtmp", 0, TheModule.get());

            if (holds_alternative<double>(value))
                return llvm::ConstantFP::get(*TheContext, llvm::APFloat(std::get<double>(value)));

            if (holds_alternative<bool>(value))
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), std::get<bool>(value));

            llvm::errs() << "Unknown literal type in codegen: " << value.index() << "\n";
            return nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_LITERAL_HPP
