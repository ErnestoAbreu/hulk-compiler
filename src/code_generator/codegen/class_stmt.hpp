#ifndef HULK_CODEGEN_CLASS_HPP
#define HULK_CODEGEN_CLASS_HPP 1

#include "../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        llvm::Function* create_method(std::string type_name, function_stmt_ptr method, llvm::StructType* class_type) {
            std::vector<llvm::Type*> param_types;

            // Implicit 'self' parameter
            param_types.push_back(class_type->getPointerTo());

            for (const auto& param : method->parameters) {
                llvm::Type* param_type = GetType(param.type.get_lexeme(), TheModule.get());
                param_types.push_back(param_type);
            }

            llvm::Type* return_type = GetType(method->return_type.get_lexeme(), TheModule.get());
            llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);

            llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::InternalLinkage, type_name + "." + method->name.get_lexeme(), TheModule.get());

            int idx = 0;
            for (auto& arg : func->args()) {
                if (idx)
                    arg.setName(method->parameters[idx - 1].name.lexeme);
                else
                    arg.setName("self");
                idx++;
            }

            // Set function body
            llvm::BasicBlock* entry = llvm::BasicBlock::Create(*TheContext.get(), "entry", func);
            Builder->SetInsertPoint(entry);

            NamedValues.clear();
            for (auto& arg : func->args()) {
                llvm::AllocaInst* alloca = CreateEntryBlockAlloca(func, arg.getType(), std::string(arg.getName()));
                Builder->CreateStore(&arg, alloca);
                NamedValues[std::string(arg.getName())] = alloca;
            }

            llvm::Value* ret_val = method->body->codegen();

            Builder->CreateRet(ret_val);

            return func;
        }

        llvm::Function* create_type_constructor(const class_stmt* type_ptr, llvm::StructType* class_type) {
            std::string type_name = type_ptr->name.get_lexeme();
            std::vector<parameter> parameters = type_ptr->parameters;
            std::vector<field_stmt_ptr> fields = type_ptr->fields;

            std::vector<llvm::Type*> ctor_param_types;
            for (const auto& param : parameters) {
                llvm::Type* param_type = GetType(param.type.get_lexeme(), TheModule.get());
                ctor_param_types.push_back(param_type);
            }

            llvm::FunctionType* ctor_type = llvm::FunctionType::get(llvm::PointerType::get(class_type, 0), ctor_param_types, false);

            llvm::Function* ctor_func = llvm::Function::Create(ctor_type, llvm::Function::InternalLinkage, type_name + "._ctor", TheModule.get());

            int idx = 0;
            for (auto& arg : ctor_func->args())
                arg.setName(parameters[idx++].name.get_lexeme());

            // Set constructor body
            llvm::BasicBlock* ctor_entry = llvm::BasicBlock::Create(*TheContext.get(), "entry", ctor_func);
            Builder->SetInsertPoint(ctor_entry);

            NamedValues.clear();
            for (auto& arg : ctor_func->args()) {
                // Create an alloca for this variable.
                llvm::AllocaInst* alloca = CreateEntryBlockAlloca(ctor_func, arg.getType(), std::string(arg.getName()));
                Builder->CreateStore(&arg, alloca);
                NamedValues[std::string(arg.getName())] = alloca;
            }

            // Allocate memory for the instance
            llvm::Function* malloc_func = TheModule->getFunction("malloc");
            llvm::Value* size = llvm::ConstantExpr::getSizeOf(class_type);
            size = Builder->CreateIntCast(size, Builder->getInt64Ty(), false);
            llvm::Value* instance_raw = Builder->CreateCall(malloc_func, size, type_name + "_inst_");
            llvm::Value* instance = Builder->CreateBitCast(instance_raw, class_type->getPointerTo(), type_name + "_inst");

            // Call parent constructor
            auto parent_type_str = type_ptr->super_class->get()->name.lexeme;
            auto* parent_ctor = TheModule->getFunction(parent_type_str + "._ctor");

            std::vector<llvm::Value*> parent_args;
            for (const auto& arg : type_ptr->super_class->get()->init) {
                auto* arg_value = arg->codegen();
                parent_args.push_back(arg_value);
            }

            auto* parent_ptr = Builder->CreateCall(parent_ctor, parent_args, "parent_ptr");
            auto* type_parent_part_ptr = Builder->CreateStructGEP(class_type, instance, 0, "parent_inst_ptr");

            llvm::Function* memcpy_fn = llvm::Intrinsic::getDeclaration(
                TheModule.get(),
                llvm::Intrinsic::memcpy,
                {
                    llvm::Type::getInt8Ty(*TheContext)->getPointerTo(),
                    llvm::Type::getInt8Ty(*TheContext)->getPointerTo(),
                    Builder->getInt64Ty()
                }
            );

            llvm::Type* parent_type = GetType(parent_type_str, TheModule.get());
            llvm::Constant* Size = llvm::ConstantExpr::getSizeOf(parent_type);

            llvm::Value* dest = Builder->CreateBitCast(type_parent_part_ptr, llvm::Type::getInt8Ty(*TheContext)->getPointerTo());
            llvm::Value* src = Builder->CreateBitCast(parent_ptr, llvm::Type::getInt8Ty(*TheContext)->getPointerTo());

            Builder->CreateCall(memcpy_fn, {
                dest,
                src,
                Size,
                Builder->getInt1(false)
                });

            // Initialize the instance
            idx = 0;
            int off_set = 1;
            for (const auto& field : fields) {
                // Get field pointer
                llvm::Value* field_ptr = Builder->CreateStructGEP(class_type, instance, off_set + idx, field->name.get_lexeme() + "_ptr");
                idx++;

                // Get initializer value
                llvm::Value* init_value = field->initializer->codegen();
                if (!init_value) {
                    internal::error(field->name, "initializer is nullptr");
                    return nullptr;
                }
                Builder->CreateStore(init_value, field_ptr);
            }

            // Configurar el vptr para apuntar a la vtable
            // llvm::Value* VTablePtr = Builder->CreateStructGEP(class_type, instance, 0, "vptr_loc");
            // Builder->CreateStore(Builder->CreateBitCast(VTables[type_name], llvm::PointerType::get(VTablesType[type_name], 0)), VTablePtr);

            Builder->CreateRet(instance);

            return ctor_func;
        }

        llvm::Value* class_stmt::codegen() {
            std::string type_name = name.get_lexeme();
            llvm::StructType* class_type = llvm::StructType::create(*TheContext.get(), type_name);
            std::vector<llvm::Type*> field_types;

            // Add parent type fields at the beginning
            llvm::Type* super_type = GetType(super_class->get()->name.get_lexeme(), TheModule.get());
            field_types.push_back(super_type);

            for (const auto& field : fields) {
                llvm::Type* field_type = GetType(field->type.get_lexeme(), TheModule.get());
                field_types.push_back(field_type);
                AddStructField(type_name, field->name.lexeme);
            }

            class_type->setBody(field_types);

            // create_vtable(this, class_type);

            create_type_constructor(this, class_type);

            for (const auto& method : methods) {
                auto func = create_method(type_name, method, class_type);
                if (!func) {
                    internal::error("Code generation error in function " + method->name.lexeme + "of type " + type_name, "CODE GENERATOR");
                }
            }

            return nullptr;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_CLASS_HPP
