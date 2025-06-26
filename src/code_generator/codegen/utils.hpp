#ifndef HULK_CODEGEN_UTILS_HPP
#define HULK_CODEGEN_UTILS_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        /* VTABLE AUXILIAR METHODS */

        unsigned getMethodIndex(const std::string& type_name, const std::string& method_name) {
            const auto& methods = VTableMethodsName[type_name];
            for (unsigned i = 0; i < methods.size(); ++i) {
                if (methods[i] == method_name) {
                    return i;
                }
            }
            throw std::runtime_error("Method not found in VTable");
        }

        void registerMethod(const std::string& type_name, const std::string& parent_name, const std::string& method_name) {
            // Si es una clase nueva, inicializa su entrada
            if (VTableMethodsName.find(type_name) == VTableMethodsName.end()) {
                VTableMethodsName[type_name] = VTableMethodsName[parent_name];
            }

            // Verificar si el método ya existe (sobreescritura)
            bool overridden = false;
            for (auto& m : VTableMethodsName[type_name]) {
                if (m == method_name) {
                    overridden = true;
                    break;
                }
            }

            // Si no fue sobreescrito, añadir al final
            if (!overridden) {
                VTableMethodsName[type_name].push_back(method_name);
            }

            // Actualizar el índice global
            VTableMethodIndices[type_name + "." + method_name] = getMethodIndex(type_name, method_name);
        }

        // llvm::Value* getMethodPtr(llvm::IRBuilder<>& Builder, llvm::Value* object, const std::string& type_name, const std::string& method_name) {
        //     // 1. Obtener la vtable del objeto
        //     llvm::Value* vtablePtr = Builder.CreateStructGEP(class_type, object, 0, "vtable_ptr");
        //     llvm::Value* vtable = Builder.CreateLoad(llvm::PointerType::get(vtable_type, 0), vtablePtr);

        //     // 2. Obtener el índice del método
        //     unsigned index = getMethodIndex(type_name, method_name);

        //     // 3. Obtener el puntero al método
        //     llvm::Value* methodPtr = Builder.CreateStructGEP(vtable_type, vtable, index, "method_ptr");
        //     return Builder.CreateLoad(function_type, methodPtr, method_name + "_ptr");
        // }


        static void AddStructField(string type_name, string field_name) {
            StructFieldIndices[type_name][field_name] = StructFieldIndices[type_name].size();
        }

        static unsigned GetStructFieldIndex(string type_name, string field_name) {
            auto it = StructFieldIndices.find(type_name);
            if (it == StructFieldIndices.end()) {
                llvm::errs() << "Error: Type '" << type_name << "' not found in StructFieldIndices.\n";
                internal::error_found = true;
                return 0;
            }
            auto& field_map = it->second;
            auto field_it = field_map.find(field_name);
            if (field_it == field_map.end()) {
                llvm::errs() << "Error: Field '" << field_name << "' not found in type '" << type_name << "'.\n";
                internal::error_found = true;
                return 0;
            }

            return StructFieldIndices[type_name][field_name];
        }

        static llvm::Type* GetType(const std::string& type_name, llvm::Module* module) {
            auto& context = module->getContext();

            if (type_name == "Number")
                return llvm::Type::getDoubleTy(context);
            if (type_name == "Boolean")
                return llvm::Type::getInt1Ty(context);
            if (type_name == "String")
                return llvm::Type::getInt8Ty(context)->getPointerTo();

            return llvm::StructType::getTypeByName(context, type_name);
        }

        static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, llvm::Type* var_type, llvm::StringRef var_name) {
            llvm::IRBuilder<> TmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
            return TmpB.CreateAlloca(var_type, nullptr, var_name);
        }

        static llvm::Value* GetAllocation(var_expr* var) {
            if (!var) {
                internal::error("Variable is null in GetAllocation.");
                return nullptr;
            }

            if (var->object) {
                var_expr* object_var = dynamic_cast<var_expr*>(var->object->get());
                if (!object_var) {
                    internal::error("Object variable is not a var_expr in GetAllocation.");
                    return nullptr;
                }

                llvm::Value* object_alloca = GetAllocation(object_var);
                if (!object_alloca) {
                    internal::error("Failed to get allocation for object variable: " + object_var->name.get_lexeme());
                    return nullptr;
                }

                llvm::Type* object_type = GetType(object_var->ret_type, TheModule.get());

                llvm::Value* object_ptr = Builder->CreateLoad(object_type->getPointerTo(), object_alloca, "self_ptr");

                if (!object_ptr || !object_ptr->getType()->isPointerTy()) {
                    internal::error("Object pointer is null or not a pointer type: " + var->name.get_lexeme());
                    return nullptr;
                }

                unsigned index = GetStructFieldIndex(object_type->getStructName().str(), var->name.get_lexeme());

                llvm::Value* member_ptr = Builder->CreateStructGEP(object_type, object_ptr, index, "memberptr");
                if (!member_ptr) {
                    internal::error("Failed to create struct GEP for member: " + var->name.get_lexeme());
                    return nullptr;
                }

                return member_ptr;
            }
            else {
                auto it = NamedValues.find(var->name.get_lexeme());
                if (it == NamedValues.end()) {
                    internal::error("Variable not found in NamedValues: " + var->name.get_lexeme());
                    return nullptr;
                }

                return it->second;
            }
        }

    } // namespace code_generator
} // namespace hulk

#endif