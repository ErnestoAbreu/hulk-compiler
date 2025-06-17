#ifndef HULK_CODEGEN_BINARYEXPR_HPP
#define HULK_CODEGEN_BINARYEXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* concat(llvm::IRBuilder<>* builder, llvm::Value* str1, llvm::Value* str2) {
            llvm::Module* module = builder->GetInsertBlock()->getModule();
            auto& ctx = module->getContext();

            llvm::Function* malloc_fn = module->getFunction("malloc");
            llvm::Function* memcpy_fn = module->getFunction("llvm.memcpy.p0i8.p0i8.i64");
            llvm::Function* strlen_fn = module->getFunction("strlen");

            llvm::Value* len1 = builder->CreateCall(strlen_fn, { str1 }, "len1");
            llvm::Value* len2 = builder->CreateCall(strlen_fn, { str2 }, "len2");

            // Calcular longitud total + 1 (para el null terminator)
            llvm::Value* total_len = builder->CreateAdd(len1, len2, "total_len");
            total_len = builder->CreateAdd(total_len, builder->getInt64(1), "total_len_with_null");

            // Asignar memoria
            llvm::Value* new_str = builder->CreateCall(malloc_fn, { total_len }, "new_str");

            // Copiar primer string
            builder->CreateCall(memcpy_fn, { new_str, str1, len1, builder->getInt1(false) });

            // Copiar segundo string (después del primero)
            llvm::Value* dest_ptr = builder->CreateGEP(builder->getInt8Ty(), new_str, len1, "dest_ptr");
            builder->CreateCall(memcpy_fn, { dest_ptr, str2, len2, builder->getInt1(false) });

            // Añadir null terminator al final
            llvm::Value* null_ptr = builder->CreateGEP(builder->getInt8Ty(), new_str, total_len, "null_ptr");
            builder->CreateStore(builder->getInt8(0), null_ptr);

            return new_str;
        }

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
            case binary_op::CONCAT:
                return concat(Builder.get(), L, R);
            case binary_op::CONCAT_DOBLE:
                return concat(Builder.get(), concat(Builder.get(), L, Builder->CreateGlobalStringPtr(" ", "strtmp", 0, TheModule.get())), R);
            default:
                llvm::errs() << "Unknown binary operator\n";
                return nullptr;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_BINARYEXPR_HPP
