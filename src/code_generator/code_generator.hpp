#ifndef HULK_CODE_GENERATOR_HPP
#define HULK_CODE_GENERATOR_HPP

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include "code_generator_visitor.hpp"

namespace hulk {
    namespace code_generator {

        struct code_generator {
            std::string filename = "build/output.ll";

            code_generator() {
                // Open a new context and module.
                ast::TheContext = std::make_unique<llvm::LLVMContext>();
                ast::TheModule = std::make_unique<llvm::Module>("hulk_module", *ast::TheContext);

                // Create a new builder for the module.
                ast::Builder = std::make_unique<llvm::IRBuilder<>>(*ast::TheContext);

                create_builtin_functions();
            }

            void generate_code(const ast::program& program) {
                auto* value = program.codegen();

                std::error_code EC;
                llvm::raw_fd_ostream out(filename, EC, llvm::sys::fs::OF_None);

                if (EC) {
                    llvm::errs() << "Error al abrir el archivo: " << EC.message() << "\n";
                    return;
                }

                ast::TheModule->print(out, nullptr);
                out.close();
            }

            void create_builtin_functions() {
                add_print();
            }

            void add_print() {
                // Crear la función printf (de la biblioteca estándar de C)
                std::vector<llvm::Type*> printf_args;
                printf_args.push_back(llvm::Type::getInt8Ty(*ast::TheContext)->getPointerTo());// format string
                llvm::FunctionType* printf_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ast::TheContext), printf_args, true); // varargs

                llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf", ast::TheModule.get());

                // Crear nuestra función print
                std::vector<llvm::Type*> print_args;
                printf_args.push_back(llvm::Type::getInt8Ty(*ast::TheContext)->getPointerTo()); // Puntero genérico
                llvm::FunctionType* print_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*ast::TheContext), print_args, false);

                llvm::Function* print_func = llvm::Function::Create(print_type, llvm::Function::ExternalLinkage, "print", ast::TheModule.get());
            }
        };
    } // namespace code_generator
} // namespace hulk

#endif // HULK_CODE_GENERATOR_HPP