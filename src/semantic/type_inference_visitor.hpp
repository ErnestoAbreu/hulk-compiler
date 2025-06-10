#ifndef TYPE_INFERENCE_VISITOR_HPP
#define TYPE_INFERENCE_VISITOR_HPP

#include "context.hpp"
#include "../ast/node.hpp"

namespace hulk {
    namespace ast {
        using context = semantic::context;
        using method = semantic::method;
        using type = semantic::type;
        using string = std::string;

        string program::infer(context& ctx, const string& shouldbe_type = "") {
            do {
                ctx.semantic_errors.clear();
                ctx.repeat_infer = false;

                for (auto& stmt : statements)
                    stmt->infer(ctx);

                main->infer(ctx);
            } while (ctx.repeat_infer);
        }

        string def_type::infer(context& ctx, const string& shouldbe_type = "") {
            type& this_type = ctx.get_type(id);
            ctx.add_scope();

            for (const auto& param : params)
                ctx.add_variable(param.id, param.type);

            if (!parent.empty()) {
                type& parent_type = ctx.get_type(parent);

                for (int i = 0; i < parent_args.size(); ++i) {
                    parent_args[i]->infer(ctx, parent_type.params[i].attr_type);
                }
            }

            for (const auto& field : fields) {
                string inferred_type = field->infer(ctx);

                // Add inferred type to the field if not already set
                if (this_type.get_field(field->id).attr_type.empty())
                    this_type.get_field(field->id).attr_type = inferred_type;
            }

            for (auto& param : params)
                if (param.type.empty())
                    param.type = ctx.get_variable(param.id).attr_type;

            // Add inferred types for parameters
            for (auto& param : this_type.params) {
                if (param.attr_type.empty()) {
                    param.attr_type = ctx.get_variable(param.name).attr_type;
                }
            }

            ctx.rollback_scope();

            ctx.add_scope();

            ctx.add_variable("self", id);

            for (const auto& meth : methods) {
                meth->infer(ctx);

                // Add inferred return type and parameters to the method
                method& m = this_type.get_method(meth->id);
                if (m.return_type.empty())
                    m.return_type = meth->return_type;

                for (const auto& param : meth->params) {
                    if (m.get_param(param.id).attr_type.empty())
                        m.get_param(param.id).attr_type = param.type;
                }
            }

            ctx.rollback_scope();

            return id; // Ignore return type for type definitions
        }

        string def_field::infer(context& ctx, const string& shouldbe_type = "") {
            string inferred_type = value->infer(ctx, var_type);

            if (var_type.empty())
                var_type = inferred_type;

            return var_type;
        }

        string def_function::infer(context& ctx, const string& shouldbe_type = "") {
            ctx.add_scope();

            for (const auto& param : params)
                ctx.add_variable(param.id, param.type);

            string inferred_type = body->infer(ctx, return_type);

            if (return_type.empty())
                return_type = inferred_type;

            ctx.rollback_scope();

            return return_type;
        }

        /* Expressions */

        string block_expr::infer(context& ctx, const string& shouldbe_type = "") {
            for (int i = 0; i < exprs.size() - 1; ++i)
                exprs[i]->infer(ctx);

            return exprs.back()->infer(ctx, shouldbe_type);
        }

        string call_expr::infer(context& ctx, const string& shouldbe_type = "") {
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string method = id.substr(id.find('.') + 1);

                auto var_type = ctx.get_variable(name).attr_type;
                if (ctx.type_exists(var_type)) {
                    auto& type = ctx.get_type(var_type);
                    if (type.has_method(method)) {
                        auto& func = type.get_method(method);
                        if (args.size() != func.params.size()) {
                            // todo handle error: argument count mismatch
                        }
                        else {
                            for (int i = 0; i < args.size(); ++i) {
                                args[i]->infer(ctx, func.params[i].attr_type);
                            }

                            return func.return_type;
                        }
                    }
                    else {
                        // todo handle error: method does not exist in this type
                    }
                }
            }
            else {
                if (ctx.function_exists(id)) {
                    auto& func = ctx.get_function(id);
                    for (int i = 0; i < args.size(); ++i) {
                        args[i]->infer(ctx, func.params[i].attr_type);
                    }

                    return func.return_type;
                }
            }
            return shouldbe_type;
        }

        string let_expr::infer(context& ctx, const string& shouldbe_type = "") {
            ctx.add_scope();

            string inferred_type = value->infer(ctx, var_type);
            if (var_type.empty())
                var_type = inferred_type;

            ctx.add_variable(id, var_type);

            inferred_type = body->infer(ctx, shouldbe_type);

            ctx.rollback_scope();

            return inferred_type;
        }

        string assign_expr::infer(context& ctx, const string& shouldbe_type = "") {
            return value->infer(ctx, shouldbe_type);
        }

        string if_expr::infer(context& ctx, const string& shouldbe_type = "") {
            condition->infer(ctx, "Boolean");
            if (else_branch) {
                string then_type = then_branch->infer(ctx, shouldbe_type);
                string else_type = else_branch->infer(ctx, shouldbe_type);
                if (then_type.empty() || else_type.empty())
                    return "";
                return ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(else_type))->name;
            }
            else {
                return then_branch->infer(ctx, shouldbe_type);
            }
        }

        string while_expr::infer(context& ctx, const string& shouldbe_type = "") {
            condition->infer(ctx, "Boolean");
            return body->infer(ctx, shouldbe_type);
        }

        string binary_expr::infer(context& ctx, const string& shouldbe_type = "") {
            if (ctx.number_ops.count(op)) {
                left->infer(ctx, "Number");
                right->infer(ctx, "Number");
                return "Number";
            }

            if (ctx.boolean_ops.count(op)) {
                left->infer(ctx, "Boolean");
                right->infer(ctx, "Boolean");
                return "Boolean";
            }

            if (ctx.comparison_ops.count(op)) {
                left->infer(ctx, "Number");
                right->infer(ctx, "Number");
                return "Boolean";
            }

            if (ctx.equality_ops.count(op)) {
                left->infer(ctx);
                right->infer(ctx);
                return "Boolean";
            }

            if (ctx.string_ops.count(op)) {
                left->infer(ctx, "String");
                right->infer(ctx, "String");
                return "String";
            }

            return shouldbe_type;
        }

        string unary_expr::infer(context& ctx, const string& shouldbe_type = "") {
            if (ctx.number_ops.count(op)) {
                expr->infer(ctx, "Number");
                return "Number";
            }

            if (ctx.boolean_ops.count(op)) {
                expr->infer(ctx, "Boolean");
                return "Boolean";
            }
        }

        string variable::infer(context& ctx, const string& shouldbe_type = "") {
            if (id.find('.') != string::npos) {
                string name = id.substr(0, id.find('.'));
                string attr = id.substr(id.find('.') + 1);


                auto var_type = ctx.get_variable(name).attr_type;
                if (ctx.type_exists(var_type)) {
                    auto& type = ctx.get_type(var_type);
                    if (!type.has_field(attr)) {
                        // todo handle error: field does not exist in this type
                    }
                    return type.get_field(attr).attr_type;
                }
            }
            else {
                auto var = ctx.get_variable(id);
                if (var.attr_type.empty())
                    var.attr_type = shouldbe_type;
                return var.attr_type;
            }
            return shouldbe_type; // If not found, return the expected type
        }

        string number::infer(context& ctx, const string& shouldbe_type = "") {
            return "Number";
        }

        string boolean::infer(context& ctx, const string& shouldbe_type = "") {
            return "Boolean";
        }

        string string_::infer(context& ctx, const string& shouldbe_type = "") {
            return "String";
        }
    } // namespace ast
} // namespace hulk

#endif