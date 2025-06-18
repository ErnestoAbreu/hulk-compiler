#ifndef HULK_CODEGEN_UTILS_HPP
#define HULK_CODEGEN_UTILS_HPP 1

#include "../../ast/ast"

namespace hulk {
    namespace code_generator {

        llvm::Type* get_type(const std::string& type_name, llvm::Module* module) {
            auto& context = module->getContext();

            if (type_name == "Number")
                return llvm::Type::getDoubleTy(context);
            if (type_name == "Boolean")
                return llvm::Type::getInt1Ty(context);
            if (type_name == "String")
                return llvm::Type::getInt8Ty(context)->getPointerTo();

            // return module->getTypeByName("class." + type_name);
        }

    } // namespace code_generator
} // namespace hulk

#endif