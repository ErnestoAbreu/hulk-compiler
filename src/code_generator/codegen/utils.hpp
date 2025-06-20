#ifndef HULK_CODEGEN_UTILS_HPP
#define HULK_CODEGEN_UTILS_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace ast {

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

            if(type_name == "Object")
                return llvm::PointerType::getUnqual(context);
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
                llvm::errs() << "Error: Variable is null.\n";
                internal::error_found = true;
                return nullptr;
            }

            if (var->object) {
                var_expr* object_var = dynamic_cast<var_expr*>(var->object->get());
                if (!object_var) {
                    llvm::errs() << "Error: Object in variable is not a var_expr.\n";
                    internal::error_found = true;
                    return nullptr;
                }

                llvm::Value* object_alloca = GetAllocation(object_var);
                if (!object_alloca) {
                    llvm::errs() << "Error: Failed to get allocation for object variable.\n";
                    internal::error_found = true;
                    return nullptr;
                }

                if (!object_alloca->getType()->isPointerTy()) {
                    llvm::errs() << "Error: Object allocation is not a pointer type: " << *object_alloca->getType() << "\n";
                    internal::error_found = true;
                    return nullptr;
                }

                llvm::Type* object_type = PointerType[object_alloca];

                if (!object_type->isStructTy()) {
                    llvm::errs() << "Error: Object type is not a struct: " << *object_type << "\n";
                    internal::error_found = true;
                    return nullptr;
                }

                unsigned index = GetStructFieldIndex(object_type->getStructName().str(), var->name.get_lexeme());

                llvm::Value* member_ptr = Builder->CreateStructGEP(object_type, object_alloca, index, "memberptr");
                if (!member_ptr) {
                    llvm::errs() << "Error: Failed to create struct GEP for member: " << var->name.get_lexeme() << "\n";
                    internal::error_found = true;
                    return nullptr;
                }

                PointerType[member_ptr] = object_type->getStructElementType(index);
                return member_ptr;
            }
            else {
                auto it = NamedValues.find(var->name.get_lexeme());
                if (it == NamedValues.end()) {
                    llvm::errs() << "Error: Variable '" << var->name.get_lexeme() << "' not found.\n";
                    internal::error_found = true;
                    return nullptr;
                }

                PointerType[it->second] = it->second->getAllocatedType();
                return it->second;
            }
        }

    } // namespace code_generator
} // namespace hulk

#endif