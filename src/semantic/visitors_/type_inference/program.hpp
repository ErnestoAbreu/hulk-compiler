#ifndef HULK_SEMANTIC_TYPE_INFERENCE_PROGRAM_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_PROGRAM_HPP

#include "../../../ast/ast"
#include "utils.hpp"

namespace hulk {
    namespace ast {

        string program::infer(semantic::context& ctx, const string& shouldbe_type) const {
            do {
                semantic::infer_errors.clear();
                semantic::repeat_infer = false;

                for (auto& stmt : statements)
                    stmt->infer(ctx);

                if (main)
                    main->infer(ctx);

            } while (semantic::repeat_infer);

            for (const auto& error : semantic::infer_errors) {
                internal::error(std::get<0>(error), std::get<1>(error), std::get<2>(error));
            }

            return ""; // Ignore return type for program inference
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_SEMANTIC_TYPE_INFERENCE_PROGRAM_HPP