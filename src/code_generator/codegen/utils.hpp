#ifndef HULK_CODEGEN_UTILS_HPP
#define HULK_CODEGEN_UTILS_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        static string current_type = "";
        static string current_func = "";

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

        /* VTABLE AUXILIAR METHODS */

        static unsigned getMethodIndex(const std::string& type_name, const std::string& method_name) {
            const auto& methods = VTableMethodsName[type_name];
            for (unsigned i = 0; i < methods.size(); ++i) {
                string meth_name = methods[i].substr(methods[i].find('.') + 1);
                if (meth_name == method_name) {
                    return i;
                }
            }
            throw std::runtime_error("Method not found in VTable");
        }

        static void registerMethod(const std::string& type_name, const std::string& parent_name, const std::string& method_name) {
              // Verificar si el método ya existe (sobreescritura)
            bool overridden = false;
            for (auto& meth : VTableMethodsName[type_name]) {
                string meth_name = meth.substr(meth.find('.') + 1);
                if (meth_name == method_name) {
                    overridden = true;
                    break;
                }
            }

            // Si no fue sobreescrito, añadir al final
            if (!overridden) {
                VTableMethodsName[type_name].push_back(type_name + "." + method_name);
            }
            else {
                VTableMethodsName[type_name][getMethodIndex(type_name, method_name)] = type_name + "." + method_name;
            }
        }

        static llvm::Value* getMethodPtr(llvm::Value* object_ptr, const std::string& type_name, const std::string& method_name) {
            auto* class_type = GetType(type_name, TheModule.get());
            auto* vtable_type = VTableTypes[type_name];
            // Obtener la vtable del objeto
            llvm::Value* vtable_ptr = Builder->CreateStructGEP(class_type, object_ptr, 0, "vtable_ptr");
            llvm::Value* vtable = Builder->CreateLoad(llvm::PointerType::get(vtable_type, 0), vtable_ptr);

            // Obtener el índice del método
            unsigned index = getMethodIndex(type_name, method_name);

            auto* func_type = MethodTypes[type_name][method_name];

            // Obtener el puntero al método
            llvm::Value* method_ptr = Builder->CreateStructGEP(vtable_type, vtable, index, "method_ptr");
            // Cargar el puntero a la función
            return Builder->CreateLoad(func_type->getPointerTo(), method_ptr, method_name + "_ptr");
        }


        static void AddStructField(string type_name, string field_name) {
            StructFieldIndices[type_name][field_name] = StructFieldIndices[type_name].size() + 1;
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