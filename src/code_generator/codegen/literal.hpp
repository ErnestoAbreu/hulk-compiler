#ifndef HULK_CODEGEN_LITERAL_HPP
#define HULK_CODEGEN_LITERAL_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* literal_expr::codegen() {
            switch (value.index()) {
            case 0: // nullptr_t
                return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(*TheContext), 0));
            case 1: // string
                return Builder->CreateGlobalStringPtr(std::get<std::string>(value), "strtmp");
            case 2: // double   
                return llvm::ConstantFP::get(*TheContext, llvm::APFloat(std::get<double>(value)));
            case 3: // bool
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), std::get<bool>(value));
            default:
                llvm::errs() << "Unknown literal type in codegen: " << value.index() << "\n";
                return nullptr;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_LITERAL_HPP
