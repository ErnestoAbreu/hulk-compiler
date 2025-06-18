#ifndef HULK_CODEGEN_CLASS_HPP
#define HULK_CODEGEN_CLASS_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Function* create_type_constructor(const class_stmt* type_ptr, llvm::StructType* class_type) {
            std::string type_name = "type." + type_ptr->name.get_lexeme();
            std::vector<parameter> parameters = type_ptr->parameters;
            std::vector<field_stmt_ptr> fields = type_ptr->fields;

            std::vector<llvm::Type*> ctor_param_types;
            for (const auto& param : parameters) {
                llvm::Type* param_type = get_type(param.type.get_lexeme(), TheModule.get());
                ctor_param_types.push_back(param_type);
            }


            llvm::FunctionType* ctor_type = llvm::FunctionType::get(llvm::PointerType::get(class_type, 0), ctor_param_types, false);

            llvm::Function* ctor_func = llvm::Function::Create(ctor_type, llvm::Function::InternalLinkage, type_name + "._ctor", TheModule.get());

            // Set constructor body
            llvm::BasicBlock* ctor_entry = llvm::BasicBlock::Create(*TheContext.get(), "entry", ctor_func);
            Builder->SetInsertPoint(ctor_entry);

            int idx = 0;
            for (auto& arg : ctor_func->args())
                arg.setName(parameters[idx++].name.get_lexeme());

            NamedValues.clear();
            for (auto& arg : ctor_func->args()) {
                // Create an alloca for this variable.
                llvm::AllocaInst* alloca = CreateEntryBlockAlloca(ctor_func, arg.getType(), std::string(arg.getName()));
                Builder->CreateStore(&arg, alloca);
                NamedValues[std::string(arg.getName())] = alloca;
            }

            // Create a instance of class_type
            llvm::Value* size = llvm::ConstantExpr::getSizeOf(class_type);
            size = Builder->CreateIntCast(size, Builder->getInt64Ty(), false);

            llvm::Function* malloc_func = TheModule->getFunction("malloc");
            llvm::Value* memory = Builder->CreateCall(malloc_func, { size }, "malloc_" + type_name);

            llvm::Value* instance = Builder->CreateBitCast(memory, llvm::PointerType::get(class_type, 0), type_name + "_instance");

            //todo: initializate parent fields

            // Initialize the instance
            idx = 0;
            int off_set = 0;
            for (const auto& field : fields) {
                // Get field pointer
                llvm::Value* field_ptr = Builder->CreateStructGEP(class_type, instance, off_set + idx, field->name.get_lexeme() + "_ptr");

                // Get initializer value
                llvm::Value* init_value = field->initializer->codegen();
                if (!init_value) {
                    internal::error(field->name, "initializer is nullptr");
                    return nullptr;
                }
                Builder->CreateStore(init_value, field_ptr);
            }

            Builder->CreateRet(instance);

            return ctor_func;
        }

        llvm::Function* create_methods(const class_stmt* type_ptr, llvm::StructType* class_type) {
            std::string type_name = "type." + type_ptr->name.get_lexeme();
            auto& methods = type_ptr->methods;

            // Set methods of the class
            for (const auto& meth : methods) {
                std::vector<llvm::Type*> param_types;

                // Implicit 'self' parameter
                param_types.push_back(llvm::PointerType::get(class_type, 0));

                for (const auto& param : meth->parameters) {
                    llvm::Type* param_type = get_type(param.type.get_lexeme(), TheModule.get());
                    param_types.push_back(param_type);
                }

                llvm::Type* return_type = get_type(meth->return_type.get_lexeme(), TheModule.get());
                llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);

                llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::InternalLinkage, type_name + "." + meth->name.get_lexeme(), TheModule.get());

                // Set function body
                llvm::BasicBlock* entry = llvm::BasicBlock::Create(*TheContext.get(), "entry", func);
                Builder->SetInsertPoint(entry);

                NamedValues.clear();
                for (auto& arg : func->args()) {
                    // Create an alloca for this variable.
                    llvm::AllocaInst* alloca = CreateEntryBlockAlloca(func, arg.getType(), std::string(arg.getName()));
                    Builder->CreateStore(&arg, alloca);
                    NamedValues[std::string(arg.getName())] = alloca;
                }

                llvm::Value* ret_val = meth->body->codegen();

                Builder->CreateRet(ret_val);
            }

            return nullptr;
        }

        llvm::Value* class_stmt::codegen() {
            std::string type_name = "type." + name.get_lexeme();
            llvm::StructType* class_type = llvm::StructType::create(*TheContext.get(), type_name);
            std::vector<llvm::Type*> field_types;

            for (const auto& field : fields) {
                llvm::Type* field_type = get_type(field->type.get_lexeme(), TheModule.get());
                field_types.push_back(field_type);
            }

            //todo: handle parent fields

            class_type->setBody(field_types);

            create_type_constructor(this, class_type);

            create_methods(this, class_type);

            return nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_CLASS_HPP
