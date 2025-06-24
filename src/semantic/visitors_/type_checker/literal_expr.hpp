#ifndef HULK_SEMANTIC_TYPE_CHECKER_LITERAL_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_LITERAL_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string literal_expr::type_check(semantic::context& ctx) {
            switch (value.index()) {
            case 0:
                return "Object";
            case 1:
                return "String";
            case 2:
                return "Number";
            case 3:
                return "Boolean";
            default:
                internal::error("unknown literal type" + std::to_string(value.index()), "TYPE CHECKING");
                return "Object";
            }
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_LITERAL_EXPR_HPP