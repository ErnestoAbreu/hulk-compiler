#ifndef HULK_CODEGEN_BINARYEXPR_HPP
#define HULK_CODEGEN_BINARYEXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* power(llvm::Value* base, llvm::Value* exponent, const std::string& name) {
            // Declarar la función pow de la biblioteca matemática
            llvm::Module* module = Builder->GetInsertBlock()->getModule();
            llvm::Function* powFunc = llvm::Intrinsic::getDeclaration(
                module,
                llvm::Intrinsic::pow,
                { base->getType() }
            );

            // Llamar a la función pow
            return Builder->CreateCall(powFunc, { base, exponent }, name);
        }

        llvm::Value* binary_expr::codegen() {
            llvm::Value* L = left->codegen();
            llvm::Value* R = right->codegen();

            if (!L || !R) return nullptr;

            switch (op) {
            case binary_op::PLUS:
                return Builder->CreateFAdd(L, R, "faddtmp");
            case binary_op::MINUS:
                return Builder->CreateFSub(L, R, "subtmp");
            case binary_op::MULT:
                return Builder->CreateFMul(L, R, "multmp");
            case binary_op::DIVIDE:
                return Builder->CreateFDiv(L, R, "divtmp");
            case binary_op::MODULE:
                return Builder->CreateFRem(L, R, "modtmp");
            case binary_op::EXPONENT:
                return power(L, R, "powtmp");
            case binary_op::GREATER:
                return Builder->CreateFCmpOGT(L, R, "fcmpgt");
            case binary_op::GREATER_EQUAL:
                return Builder->CreateFCmpOGE(L, R, "fcmpge");
            case binary_op::LESS:
                return Builder->CreateFCmpOLT(L, R, "fcmplt");
            case binary_op::LESS_EQUAL:
                return Builder->CreateFCmpOLE(L, R, "fcmple");
            case binary_op::NOT_EQUAL:
                return Builder->CreateFCmpONE(L, R, "fcmpne");
            case binary_op::EQUAL_EQUAL:
                return Builder->CreateFCmpOEQ(L, R, "eqtmp");
            case binary_op::OR:
                return Builder->CreateOr(L, R, "ortmp");
            case binary_op::AND:
                return Builder->CreateAnd(L, R, "andtmp");
            default:
                llvm::errs() << "Unknown binary operator\n";
                return nullptr;
            }

            
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_BINARYEXPR_HPP
