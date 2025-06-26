#ifndef HULK_CODEGEN_BINARYEXPR_HPP
#define HULK_CODEGEN_BINARYEXPR_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* doube_to_string(llvm::Value* value) {
            llvm::Module* module = Builder->GetInsertBlock()->getModule();

            llvm::Function* sprintfFunc = module->getFunction("sprintf"); // Buscar la función sprintf de C en el módulo
            if (!sprintfFunc) {
                llvm::FunctionType* sprintfType = llvm::FunctionType::get(
                    Builder->getInt32Ty(),
                    {
                        Builder->getInt8Ty()->getPointerTo(), // Buffer
                        Builder->getInt8Ty()->getPointerTo(), // Format string
                    },
                    true // Is variadic
                    );
                sprintfFunc = llvm::Function::Create(
                    sprintfType,
                    llvm::Function::ExternalLinkage,
                    "sprintf",
                    module
                );
            }

            llvm::Value* buffer = Builder->CreateAlloca(Builder->getInt8Ty(), Builder->getInt32(64));

            llvm::Value* formatStr = Builder->CreateGlobalStringPtr("%.6g"); // 6 dígitos de precisión

            Builder->CreateCall(sprintfFunc, { buffer, formatStr, value });

            return buffer;
        }

        llvm::Value* concat(llvm::IRBuilder<>* builder, llvm::Value* str1, llvm::Value* str2) {
            if (str1->getType()->isDoubleTy()) {
                str1 = doube_to_string(str1);
            }
            if (str2->getType()->isDoubleTy()) {
                str2 = doube_to_string(str2);
            }

            llvm::Module* module = builder->GetInsertBlock()->getModule();
            auto& ctx = module->getContext();

            // 1. Obtener funciones necesarias (con nombres actualizados)   
            llvm::Function* malloc_fn = module->getFunction("malloc");

            // Usar la declaración correcta de memcpy (¡clave!)
            llvm::Function* memcpy_fn = llvm::Intrinsic::getDeclaration(
                module,
                llvm::Intrinsic::memcpy,
                { builder->getInt8Ty()->getPointerTo(), builder->getInt8Ty()->getPointerTo(), builder->getInt64Ty() }
            );

            llvm::Function* strlen_fn = module->getFunction("strlen");

            // 2. Calcular longitudes
            llvm::Value* len1 = builder->CreateCall(strlen_fn, { str1 }, "len1");
            llvm::Value* len2 = builder->CreateCall(strlen_fn, { str2 }, "len2");

            // 3. Calcular longitud total + null terminator
            llvm::Value* total_len = builder->CreateAdd(len1, len2, "total_len");
            total_len = builder->CreateAdd(total_len, builder->getInt64(1), "total_len_with_null");

            // 4. Asignar memoria
            llvm::Value* new_str = builder->CreateCall(malloc_fn, { total_len }, "new_str");
            new_str = builder->CreateBitCast(new_str, builder->getInt8Ty()->getPointerTo()); // Asegurar tipo i8*

            // 5. Copiar strings
            llvm::Value* is_volatile = builder->getInt1(false);

            // Primer string
            builder->CreateCall(memcpy_fn, { new_str,builder->CreateBitCast(str1, builder->getInt8Ty()->getPointerTo()),len1,is_volatile });

            // Segundo string
            llvm::Value* dest_ptr = builder->CreateGEP(builder->getInt8Ty(), new_str, len1, "dest_ptr");
            builder->CreateCall(memcpy_fn, { dest_ptr, builder->CreateBitCast(str2, builder->getInt8Ty()->getPointerTo()), len2, is_volatile });

            // 6. Null terminator
            llvm::Value* null_ptr = builder->CreateGEP(builder->getInt8Ty(), new_str, total_len, "null_ptr");
            builder->CreateStore(builder->getInt8(0), null_ptr);

            return new_str;
        }

        llvm::Value* power(llvm::Value* base, llvm::Value* exponent, const std::string& name) {
            // Declarar la función pow de la biblioteca matemática
            llvm::Module* module = Builder->GetInsertBlock()->getModule();
            llvm::Function* powFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::pow, { base->getType() });

            // Llamar a la función pow
            return Builder->CreateCall(powFunc, { base, exponent }, name);
        }

        llvm::Value* binary_expr::codegen() {
            llvm::Value* L = left->codegen();
            llvm::Value* R = right->codegen();

            if (!L || !R) {
                internal::error(token, "binary expression is nullptr");
                return nullptr;
            }

            switch (op) {
            case binary_op::PLUS:
                return Builder->CreateFAdd(L, R, "faddtmp");
            case binary_op::MINUS:
                return Builder->CreateFSub(L, R, "fsubtmp");
            case binary_op::MULT:
                return Builder->CreateFMul(L, R, "fmultmp");
            case binary_op::DIVIDE:
                return Builder->CreateFDiv(L, R, "fdivtmp");
            case binary_op::MODULE:
                return Builder->CreateFRem(L, R, "fmodtmp");
            case binary_op::EXPONENT:
                return power(L, R, "fpowtmp");
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
                internal::error(token, "unknown binary operator");
                return nullptr;
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_BINARYEXPR_HPP
