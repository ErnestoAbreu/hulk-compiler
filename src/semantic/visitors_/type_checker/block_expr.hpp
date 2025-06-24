#ifndef HULK_SEMANTIC_TYPE_CHECKER_BLOCK_EXPR_HPP
#define HULK_SEMANTIC_TYPE_CHECKER_BLOCK_EXPR_HPP

#include "../../../ast/ast"

namespace hulk {
    namespace ast {

        string block_expr::type_check(semantic::context& ctx) {
            string last_type = "Object";

            for (int i = 0; i < expressions.size(); ++i)
                last_type = expressions[i]->type_check(ctx);

            return last_type;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_CHECKER_BLOCK_EXPR_HPP