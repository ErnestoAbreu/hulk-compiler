#ifndef CODE_GENERATOR_VISITOR_HPP
#define CODE_GENERATOR_VISITOR_HPP 1

#include <map>

#include "../ast/ast"

namespace hulk {
    namespace ast {
        static std::unique_ptr<llvm::LLVMContext> TheContext;
        static std::unique_ptr<llvm::Module> TheModule;
        static std::unique_ptr<llvm::IRBuilder<>> Builder;
        static std::map<std::string, llvm::Value*> NamedValues;

        llvm::Value* program::codegen() const {
            // Generate code for each statement in the program
            for (const auto& stmt : statements)
                if (!stmt->codegen())
                    return nullptr;

            // Crear la función main
            std::vector<llvm::Type*> main_args;
            llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ast::TheContext), main_args, false);

            llvm::Function* main_func = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", ast::TheModule.get());

            // Crear un bloque básico para la función main
            llvm::BasicBlock* entry = llvm::BasicBlock::Create(*ast::TheContext, "entry", main_func);
            ast::Builder->SetInsertPoint(entry);

            // Generate code for the main expression
            if (main) {
                auto* value = main->codegen();
                // Retornar 0 al final de la función main
                ast::Builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ast::TheContext), 0));

                return value;
            }

            return nullptr;
        }

        llvm::Value* binary_expr::codegen() {
            llvm::Value* L = left->codegen();
            llvm::Value* R = right->codegen();

            if (!L || !R) return nullptr;

            switch (op) {
            case binary_op::PLUS:
                return Builder->CreateFAdd(L, R, "addtmp");
            case binary_op::MINUS:
                return Builder->CreateSub(L, R, "subtmp");
            case binary_op::MULT:
                return Builder->CreateMul(L, R, "multmp");
            case binary_op::DIVIDE:
                return Builder->CreateSDiv(L, R, "divtmp");
            case binary_op::GREATER:
                return Builder->CreateICmpSGT(L, R, "gttmp");
            case binary_op::GREATER_EQUAL:
                return Builder->CreateICmpSGE(L, R, "getmp");
            case binary_op::LESS:
                return Builder->CreateICmpSLT(L, R, "lestmp");
            case binary_op::LESS_EQUAL:
                return Builder->CreateICmpSLE(L, R, "letmp");
            case binary_op::NOT_EQUAL:
                return Builder->CreateICmpNE(L, R, "netmp");
            case binary_op::EQUAL_EQUAL:
                return Builder->CreateICmpEQ(L, R, "eqtmp");
            case binary_op::OR:
                return Builder->CreateOr(L, R, "ortmp");
            case binary_op::AND:
                return Builder->CreateAnd(L, R, "andtmp");
            default:
                return nullptr;
            }
        }

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

        llvm::Value* call_expr::codegen() {
            // Special case for print
            if (callee.lexeme == "print") {
                if (arguments.size() != 1) {
                    llvm::errs() << "print function expects exactly one argument\n";
                    return nullptr;
                }

                llvm::Value* ArgValue = arguments[0]->codegen();
                if (!ArgValue)
                    return nullptr;

                // Create format string
                llvm::Value* FormatStr = Builder->CreateGlobalStringPtr("%f\n", "fmt", 0, TheModule.get());

                // Call printf
                std::vector<llvm::Value*> PrintfArgs;
                PrintfArgs.push_back(FormatStr);
                PrintfArgs.push_back(ArgValue);

                llvm::Function* PrintfF = TheModule->getFunction("printf");
                return Builder->CreateCall(PrintfF, PrintfArgs, "printfcall");
            }

            // Function* CalleeF = TheModule->getFunction(Callee);
            // if (!CalleeF)
            //     return LogErrorV("Unknown function referenced");

            // if (CalleeF->arg_size() != Args.size())
            //     return LogErrorV("Incorrect # arguments passed");

            // std::vector<Value*> ArgsV;
            // for (unsigned i = 0, e = Args.size(); i != e; ++i) {
            //     ArgsV.push_back(Args[i]->codegen());
            //     if (!ArgsV.back())
            //         return nullptr;
            // }

            // return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
            return nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // CODE_GENERATOR_VISITOR_HPP