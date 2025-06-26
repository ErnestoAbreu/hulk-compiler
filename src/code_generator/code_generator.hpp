#ifndef HULK_CODE_GENERATOR_HPP
#define HULK_CODE_GENERATOR_HPP

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include "codegen_"

namespace hulk {
    namespace code_generator {

        struct code_generator {
            std::string filename = "hulk/output.ll";

            code_generator() {
                // Open a new context and module.
                ast::TheContext = std::make_unique<llvm::LLVMContext>();
                ast::TheModule = std::make_unique<llvm::Module>("hulk_module", *ast::TheContext);

                // Create a new builder for the module.
                ast::Builder = std::make_unique<llvm::IRBuilder<>>(*ast::TheContext);

                ast::NamedValues.clear();

                create_builtin_types();
                create_builtin_functions();
            }

            int generate_code(const ast::program& program) {
                program.codegen();

                if (internal::error_found)
                    return -1;

                if (llvm::verifyModule(*ast::TheModule, &llvm::errs())) {
                    llvm::errs() << "Error: El módulo contiene IR inválido\n";
                    // return -1;
                }

                std::error_code EC;
                llvm::raw_fd_ostream out(filename, EC, llvm::sys::fs::OF_None);

                if (EC) {
                    llvm::errs() << "Error al abrir el archivo: " << EC.message() << "\n";
                    return -1;
                }

                ast::TheModule->print(out, nullptr);
                out.close();
                return 0;
            }

            void create_builtin_types() {
                create_object();
            }

            void create_object() {
                auto& context = *ast::TheContext;

                llvm::StructType* object = llvm::StructType::create(context, "Object");

                llvm::Type* vtable_type = llvm::ArrayType::get(
                    llvm::PointerType::getUnqual(
                        llvm::FunctionType::get(llvm::Type::getVoidTy(context), true)
                    ),
                    1
                );

                object->setBody(vtable_type->getPointerTo());

                llvm::FunctionType* ctor_type = llvm::FunctionType::get(llvm::PointerType::get(object, 0), {}, false);
                llvm::Function* ctor_func = llvm::Function::Create(ctor_type, llvm::Function::InternalLinkage, "Object._ctor", ast::TheModule.get());

                llvm::BasicBlock* ctor_entry = llvm::BasicBlock::Create(*ast::TheContext.get(), "entry", ctor_func);
                ast::Builder->SetInsertPoint(ctor_entry);

                llvm::Value* object_ptr = ast::Builder->CreateAlloca(object, nullptr, "object_ptr");
                ast::Builder->CreateRet(object_ptr);
            }

            void create_builtin_functions() {
                declare_printf();
                declare_malloc();
                declare_string_helpers();
            }

            void declare_printf() {
                // Declare printf function
                std::vector<llvm::Type*> printf_args;
                printf_args.push_back(llvm::Type::getInt8Ty(*ast::TheContext)->getPointerTo());// format string
                llvm::FunctionType* printf_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ast::TheContext), printf_args, true); // varargs

                llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf", ast::TheModule.get());
            }

            void declare_malloc() {
                llvm::FunctionType* malloc_type = llvm::FunctionType::get(
                    llvm::Type::getInt8Ty(*ast::TheContext)->getPointerTo(), { llvm::Type::getInt64Ty(*ast::TheContext) }, false
                );
                llvm::Function::Create(malloc_type, llvm::Function::ExternalLinkage, "malloc", ast::TheModule.get());
            }

            void declare_string_helpers() {
                auto& ctx = ast::TheModule->getContext();

                // Declarar memcpy
                llvm::FunctionType* memcpy_type = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {
                        llvm::Type::getInt8Ty(ctx)->getPointerTo(),
                        llvm::Type::getInt8Ty(ctx)->getPointerTo(),
                        llvm::Type::getInt64Ty(ctx),
                        llvm::Type::getInt1Ty(ctx)
                    },
                    false
                );
                llvm::Function::Create(memcpy_type, llvm::Function::ExternalLinkage, "llvm.memcpy.p0i8.p0i8.i64", ast::TheModule.get());

                // Declarar strlen
                llvm::FunctionType* strlen_type = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(ctx), { llvm::Type::getInt8Ty(ctx)->getPointerTo() }, false
                );
                llvm::Function::Create(strlen_type, llvm::Function::ExternalLinkage, "strlen", ast::TheModule.get());
            }
        };

    } // namespace code_generator
} // namespace hulk

#endif // HULK_CODE_GENERATOR_HPP