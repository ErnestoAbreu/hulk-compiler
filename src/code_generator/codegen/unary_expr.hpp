#ifndef HULK_CODEGEN_UNARYEXPR_HPP
#define HULK_CODEGEN_UNARYEXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {
        
        llvm::Value* unary_expr::codegen() {
            llvm::Value* V = expression->codegen();

            if (!V) return nullptr;

            switch (op) {
            case unary_op::MINUS:
                return Builder->CreateNeg(V, "negtmp");
            case unary_op::NOT:
                return Builder->CreateNot(V, "nottmp");
            default:
                return nullptr;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_UNARYEXPR_HPP
