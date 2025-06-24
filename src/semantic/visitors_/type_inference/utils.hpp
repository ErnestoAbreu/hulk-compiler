#ifndef HULK_SEMANTIC_TYPE_INFERENCE_UTILS_HPP
#define HULK_SEMANTIC_TYPE_INFERENCE_UTILS_HPP

#include <set>
#include <tuple>
#include <string>
#include <vector>
#include "../../../ast/enums"

namespace hulk {
    namespace semantic {

        static std::vector<std::tuple<int, int, std::string>> infer_errors;
        static bool repeat_infer;

        static void add_infer_error(int line, int column, const std::string& message) {
            infer_errors.emplace_back(line, column, message);
        }

        static std::set<ast::binary_op> number_ops = {
            ast::binary_op::PLUS, ast::binary_op::MINUS, ast::binary_op::MULT,
            ast::binary_op::DIVIDE, ast::binary_op::MODULE, ast::binary_op::EXPONENT
        }; //  number <op> number => number

        static std::set<ast::binary_op> boolean_ops = {
            ast::binary_op::OR, ast::binary_op::AND
        }; // boolean <op> boolean => boolean

        static std::set<ast::binary_op> comparison_ops = {
            ast::binary_op::GREATER, ast::binary_op::GREATER_EQUAL,
            ast::binary_op::LESS, ast::binary_op::LESS_EQUAL
        }; // number <op> number => boolean

        static std::set<ast::binary_op> equality_ops = {
            ast::binary_op::EQUAL_EQUAL, ast::binary_op::NOT_EQUAL
        }; // any <op> any => boolean

        static std::set<ast::binary_op> string_ops = {
            ast::binary_op::CONCAT, ast::binary_op::CONCAT_DOBLE
        }; // string <op> string => string || number <op> string => string || string <op> number => string

    } // namespace semantic
} // namespace hulk

#endif