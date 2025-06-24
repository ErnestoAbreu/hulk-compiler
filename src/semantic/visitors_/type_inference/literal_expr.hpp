#ifndef HULK_SEMANTIC_TYPE_INFERENCE_LITERAL_EXPR_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_LITERAL_EXPR_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string literal_expr::infer(semantic::context& ctx, const string& shouldbe_type) {
            if (value.index() == 0)
                return "Null"; // nullptr
            else if (value.index() == 1)
                return "String"; // std::string
            else if (value.index() == 2)
                return "Number"; // double
            else if (value.index() == 3)
                return "Boolean"; // bool
            else
                return shouldbe_type; // If type is not recognized, return the expected type
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_PROGRAM_HPP