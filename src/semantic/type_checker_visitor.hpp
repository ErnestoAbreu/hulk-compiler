#ifndef TYPE_CHECKER_VISITOR_HPP
#define TYPE_CHECKER_VISITOR_HPP

#include "context.hpp"
#include "../ast/node.hpp"

namespace hulk {
    namespace ast {
        using context = semantic::context;
        using type = semantic::type;

        void program::type_check(context& ctx) const {
            for (const auto& stmt : statements) {
                stmt->type_check(ctx);
            }
            if (main)
                main->type_check(ctx);
        }

        void def_type::type_check(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : params) {
                ctx.add_variable(param.id, param.type);
            }

            if (!parent.empty()) {
                auto& parent_type = ctx.get_type(parent);
                for (int i = 0; i < parent_args.size(); ++i) {
                    auto* arg_type = parent_args[i]->type_check(ctx);
                    if (*arg_type <= ctx.get_type(parent_type.params[i].attr_type));
                    else {
                        // todo handle error: argument type does not match parameter type
                    }
                }
            }

            for (const auto& field : fields) {
                field->type_check(ctx);
            }

            ctx.rollback_scope();

            ctx.add_scope();

            ctx.add_variable("self", id);

            for (const auto& method : methods) {
                method->type_check(ctx);
            }

            ctx.rollback_scope();
        }

        void def_function::type_check(context& ctx) const {
            ctx.add_scope();

            for (const auto& param : params) {
                ctx.add_variable(param.id, param.type);
            }

            type* body_type = body->type_check(ctx);
            if (*body_type <= ctx.get_type(return_type));
            else {
                // todo handle error: body type does not match return type
            }

            ctx.rollback_scope();
        }

        void def_field::type_check(context& ctx) const {
            type* value_type = value->type_check(ctx);
            type field_type = ctx.get_type(var_type);
            if (*value_type <= field_type);
            else {
                // todo handle error: value type does not match field type
            }
        }

        type* ast::block_expr::type_check(context& ctx) {
            type* last_type;
            for (const auto& expr : exprs) {
                last_type = expr->type_check(ctx);
            }
            return return_type = last_type;
        }

        type* ast::call_expr::type_check(context& ctx) {
            method func;
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string attr = id.substr(id.find('.') + 1);

                auto var = ctx.get_variable(name);
                type var_type = ctx.get_type(var.attr_type);
                if (var_type.has_method(attr)) {
                    func = var_type.get_method(attr);
                }
                else {
                    // todo handle error: method not found
                    return &ctx.get_type("?undefined");
                }
            }
            else {
                if (ctx.function_exists(id)) {
                    func = ctx.get_function(id);
                }
            }

            if (args.size() != func.params.size()) {
                // todo handle error: wrong number of arguments
            }
            else {
                for (size_t i = 0; i < args.size(); ++i) {
                    type* arg_type = args[i]->type_check(ctx);
                    if (*arg_type <= ctx.get_type(func.params[i].attr_type));
                    else {
                        // todo handle error: argument type does not match parameter type
                    }
                }
            }

            return return_type = &ctx.get_type(func.return_type);
        }

        type* ast::let_expr::type_check(context& ctx) {
            ctx.add_scope();

            type* value_type = value->type_check(ctx);
            if (*value_type <= ctx.get_type(var_type)) {
                ctx.add_variable(id, var_type);
            }
            else {
                // todo handle error: value type does not match variable type
            }

            type* body_type = body->type_check(ctx);

            ctx.rollback_scope();

            return return_type = body_type;
        }

        type* ast::assign_expr::type_check(context& ctx) {
            type* value_type = value->type_check(ctx);
            type* var_type = &ctx.get_type("?undefined");

            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string attr = id.substr(id.find('.') + 1);

                if (name == "self") {
                    auto var_type_str = ctx.get_variable(name).attr_type;
                    if (ctx.type_exists(var_type_str)) {
                        var_type = &ctx.get_type(var_type_str);
                        type& field_type = ctx.get_type(var_type->get_field(attr).attr_type);
                        if (*value_type <= field_type);
                        else {
                            // todo handle error: value type does not match field type
                        }
                        var_type = &field_type;
                    }
                }
            }
            else {
                auto& var = ctx.get_variable(id);
                var_type = &ctx.get_type(var.attr_type);

                if (*value_type <= *var_type);
                else {
                    // todo handle error: value type does not match variable type
                }
            }

            return return_type = var_type;
        }

        type* ast::if_expr::type_check(context& ctx) {
            type* cond_type = condition->type_check(ctx);
            if (*cond_type == ctx.get_type("Boolean")) {
                type* then_type = then_branch->type_check(ctx);
                if (else_branch) {
                    type* else_type = else_branch->type_check(ctx);
                    type* lca_type = ctx.get_lca_type(then_type, else_type);
                    return return_type = lca_type;
                }
                else {
                    return return_type = then_type;
                }
            }
            else {
                // todo handle error: condition is not a boolean
            }
            return &ctx.get_type("?undefined");
        }

        type* ast::while_expr::type_check(context& ctx) {
            type* cond_type = condition->type_check(ctx);
            if (*cond_type == ctx.get_type("Boolean")) {
                return return_type = body->type_check(ctx);
            }
            else {
                // todo handle error: condition is not a boolean
            }
            return &ctx.get_type("?undefined");
        }

        type* ast::binary_expr::type_check(context& ctx) {
            type* left_type = left->type_check(ctx);
            type* right_type = right->type_check(ctx);

            if (ctx.number_ops.count(op)) {
                if (*left_type != ctx.get_type("Number")) {
                    // todo handle error: left operand is not a number
                }
                if (*right_type != ctx.get_type("Number")) {
                    // todo handle error: right operand is not a number
                }
                return return_type = &ctx.get_type("Number");
            }

            if (ctx.boolean_ops.count(op)) {
                if (*left_type != ctx.get_type("Boolean")) {
                    // todo handle error: left operand is not a boolean
                }
                if (*right_type != ctx.get_type("Boolean")) {
                    // todo handle error: right operand is not a boolean
                }
                return return_type = &ctx.get_type("Boolean");
            }

            if (ctx.comparison_ops.count(op)) {
                if (*left_type != ctx.get_type("Number")) {
                    // todo handle error: left operand is not a number
                }
                if (*right_type != ctx.get_type("Number")) {
                    // todo handle error: right operand is not a number
                }
                return return_type = &ctx.get_type("Boolean");
            }

            if (ctx.equality_ops.count(op)) {
                return return_type = &ctx.get_type("Boolean");
            }

            if (ctx.string_ops.count(op)) {
                if (*left_type != ctx.get_type("String")) {
                    // todo handle error: left operand is not a string
                }
                if (*right_type != ctx.get_type("String")) {
                    // todo handle error: right operand is not a string
                }
                return return_type = &ctx.get_type("String");
            }

            // todo handle error: unknown operator
            return &ctx.get_type("?undefined");
        }

        type* ast::unary_expr::type_check(context& ctx) {
            type* expr_type = expr->type_check(ctx);

            if (ctx.number_ops.count(op)) {
                if (*expr_type != ctx.get_type("Number")) {
                    // todo handle error: operand is not a number
                }
                return return_type = &ctx.get_type("Number");
            }

            if (ctx.boolean_ops.count(op)) {
                if (*expr_type != ctx.get_type("Boolean")) {
                    // todo handle error: operand is not a boolean
                }
                return return_type = &ctx.get_type("Boolean");
            }

            // todo handle error: unknown operator
            return &ctx.get_type("?undefined");
        }

        type* ast::variable::type_check(context& ctx) {
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string attr = id.substr(id.find('.') + 1);

                if (name == "self") {
                    string var_type_str = ctx.get_variable(name).attr_type;
                    type& var_type = ctx.get_type(var_type_str);
                    return return_type = &ctx.get_type(var_type.get_field(attr).attr_type);
                }
                return &ctx.get_type("?undefined");
            }
            else {
                auto var = ctx.get_variable(id);
                return return_type = &ctx.get_type(var.attr_type);
            }
        }

        type* ast::number::type_check(context& ctx) {
            return return_type = &ctx.get_type("Number");
        }

        type* ast::boolean::type_check(context& ctx) {
            return return_type = &ctx.get_type("Boolean");
        }

        type* ast::string_::type_check(context& ctx) {
            return return_type = &ctx.get_type("String");
        }
    } // namespace ast
} // namespace hulk

#endif //TYPE_CHECKER_VISITOR_HPP