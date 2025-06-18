#ifndef TYPE_INFERENCE_VISITOR_HPP
#define TYPE_INFERENCE_VISITOR_HPP

#include "../context"
#include "../../ast/ast"

namespace hulk {
    namespace ast {
        using context = semantic::context;
        using method = semantic::method;
        using type = semantic::type;
        using string = std::string;

        string program::infer(context& ctx, const string& shouldbe_type) const {
            do {
                ctx.infer_errors.clear();
                ctx.repeat_infer = false;

                for (auto& stmt : statements)
                    stmt->infer(ctx);

                if (main)
                    main->infer(ctx);

            } while (ctx.repeat_infer);

            for (const auto& error : ctx.infer_errors) {
                internal::semantic_error(std::get<0>(error), std::get<1>(error), std::get<2>(error));
            }

            return ""; // Ignore return type for program inference
        }

        string class_stmt::infer(context& ctx, const string& shouldbe_type) {
            type& this_type = ctx.get_type(name.lexeme);

            ctx.add_scope();

            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            if (!super_class.has_value()) {
                type& parent_type = ctx.get_type(super_class.value()->name.lexeme);

                auto& parent_args = super_class.value()->init;
                for (int i = 0; i < parent_args.size(); ++i) {
                    parent_args[i]->infer(ctx, parent_type.params[i].attr_type);
                }
            }

            for (const auto& field : fields) {
                string inferred_type = field->infer(ctx);

                // Add inferred type to the field if not already set
                if (this_type.get_field(field->name.lexeme).attr_type.empty()) {
                    if (inferred_type.empty()) {
                        ctx.add_infer_error(field->name.line, field->name.column,
                            "Field '" + field->name.lexeme + "' type of Type '" + name.lexeme + "' was not inferred, is necessary to annotate it.");
                    }
                    else {
                        field->type.lexeme = inferred_type;
                        ctx.repeat_infer = true; // If type was inferred, we need to repeat inference
                    }
                }
            }

            for (auto& param : parameters)
                if (param.type.lexeme.empty())
                    param.type.lexeme = ctx.get_variable(param.name.lexeme).attr_type;

            // Add inferred types for parameters
            for (auto& param : this_type.params)
                if (param.attr_type.empty())
                    param.attr_type = ctx.get_variable(param.name).attr_type;

            ctx.rollback_scope();

            ctx.add_scope();

            ctx.add_variable("self", name.lexeme);

            for (const auto& meth : methods) {
                meth->infer(ctx);

                // Add inferred return type and parameters to the method
                method& m = this_type.get_method(meth->name.lexeme);
                if (m.return_type.empty())
                    m.return_type = meth->return_type.lexeme;

                for (const auto& param : meth->parameters) {
                    if (m.get_param(param.name.lexeme).attr_type.empty())
                        m.get_param(param.name.lexeme).attr_type = param.type.lexeme;
                }
            }

            ctx.rollback_scope();

            return name.lexeme; // Ignore return type for type definitions
        }

        string field_stmt::infer(context& ctx, const string& shouldbe_type) {
            string inferred_type = initializer->infer(ctx, type.lexeme);

            if (type.lexeme.empty()) {
                type.lexeme = inferred_type;
                ctx.repeat_infer = true; // If type was inferred, we need to repeat inference
            }

            return type.lexeme;
        }

        string function_stmt::infer(context& ctx, const string& shouldbe_type) {
            ctx.add_scope();

            auto& func = ctx.get_function(name.lexeme);

            for (const auto& param : parameters)
                ctx.add_variable(param.name.lexeme, param.type.lexeme);

            string inferred_type = body->infer(ctx, return_type.lexeme);

            if (return_type.lexeme.empty()) {
                if (inferred_type.empty()) {
                    ctx.add_infer_error(name.line, name.column,
                        "Return type for function '" + name.lexeme + "' was not inferred, is necessary to annotate it.");
                }
                else {
                    return_type.lexeme = inferred_type;
                    ctx.repeat_infer = true; // If return type was inferred, we need to repeat inference
                }
            }

            if (func.return_type.empty()) {
                func.return_type = return_type.lexeme;
            }

            for (auto& param : parameters) {
                if (func.get_param(param.name.lexeme).attr_type.empty()) {
                    inferred_type = ctx.get_variable(param.name.lexeme).attr_type;
                    param.type.lexeme = inferred_type;
                    func.get_param(param.name.lexeme).attr_type = inferred_type;
                }
            }

            ctx.rollback_scope();

            return return_type.lexeme;
        }

        /* Expressions */

        string block_expr::infer(context& ctx, const string& shouldbe_type) {
            for (int i = 0; i < expressions.size() - 1; ++i)
                expressions[i]->infer(ctx);

            return expressions.back()->infer(ctx, shouldbe_type);
        }

        string call_expr::infer(context& ctx, const string& shouldbe_type) {
            if (object.has_value()) {
                string object_type = object.value()->infer(ctx);

                if (object_type.empty()) {
                    ctx.add_infer_error(callee.line, callee.column,
                        "Object type for method call is not inferred.");
                    return shouldbe_type; // If object type is not inferred, return expected type
                }

                auto& type = ctx.get_type(object_type);

                if (!type.has_method(callee.lexeme)) {
                    ctx.add_infer_error(callee.line, callee.column,
                        "Method '" + callee.lexeme + "' does not exist in type '" + object_type + "'.");
                    return shouldbe_type; // If method does not exist, return expected type
                }

                auto& method = type.get_method(callee.lexeme);
                if (method.params.size() != arguments.size()) {
                    ctx.add_infer_error(callee.line, callee.column,
                        "Method '" + callee.lexeme + "' expects " + std::to_string(method.params.size()) +
                        " arguments, but got " + std::to_string(arguments.size()) + ".");
                    return shouldbe_type; // If argument count does not match, return expected type
                }

                for (size_t i = 0; i < arguments.size(); ++i) {
                    arguments[i]->infer(ctx, method.params[i].attr_type);
                }

                if (method.return_type.empty())
                    return shouldbe_type; // If method return type is not defined, return expected type
                else
                    return method.return_type; // Return the method's return type
            }
            else {
                auto& func = ctx.get_function(callee.lexeme);

                for (size_t i = 0; i < arguments.size(); ++i) {
                    arguments[i]->infer(ctx, func.params[i].attr_type);
                }

                if (func.return_type.empty())
                    return shouldbe_type; // If function return type is not defined, return expected type
                else
                    return func.return_type; // Return the function's return type
            }
        }

        string let_expr::infer(context& ctx, const string& shouldbe_type) {
            for (const auto& assignment : assignments) {
                ctx.add_scope();

                assignment->infer(ctx);
            }

            string inferred_type = body->infer(ctx, shouldbe_type);

            for (const auto& assignment : assignments) {
                ctx.rollback_scope();
            }

            return inferred_type;
        }

        string declaration_expr::infer(context& ctx, const string& shouldbe_type) {
            if (value) {
                string inferred_type = value->infer(ctx, type.lexeme);

                if (type.lexeme.empty()) {
                    type.lexeme = inferred_type;
                    ctx.repeat_infer = true;
                }
            }

            ctx.add_variable(name.lexeme, type.lexeme);

            if (type.lexeme.empty()) {
                ctx.add_infer_error(name.line, name.column,
                    "Type for variable '" + name.lexeme + "' was no inferred, is necessary to annotate it.");
                return shouldbe_type; // If type is not inferred, return the expected type
            }

            return type.lexeme;
        }

        string assign_expr::infer(context& ctx, const string& shouldbe_type) {
            string inferred_type = variable->infer(ctx, shouldbe_type);
            return value->infer(ctx, inferred_type);
        }

        string if_expr::infer(context& ctx, const string& shouldbe_type) {
            condition->infer(ctx, "Boolean");

            string then_type = then_branch->infer(ctx, shouldbe_type);

            for (const auto& elif : elif_branchs) {
                elif.first->infer(ctx, "Boolean");

                string elif_type = elif.second->infer(ctx, shouldbe_type);

                if (then_type.empty() || elif_type.empty());
                else
                    then_type = ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(elif_type))->name;
            }

            if (else_branch.has_value()) {
                string else_type = else_branch.value()->infer(ctx, shouldbe_type);

                if (then_type.empty() || else_type.empty());
                else
                    then_type = ctx.get_lca_type(&ctx.get_type(then_type), &ctx.get_type(else_type))->name;
            }

            if (then_type.empty())
                then_type = shouldbe_type;

            return then_type;
        }

        string while_expr::infer(context& ctx, const string& shouldbe_type) {
            condition->infer(ctx, "Boolean");
            return body->infer(ctx, shouldbe_type);
        }

        string for_expr::infer(context& ctx, const string& shouldbe_type) {
            string inferred_iter_type = iterable->infer(ctx, "Iterable");

            if (!inferred_iter_type.empty()) {
                auto& iter_type = ctx.get_type(inferred_iter_type);

                if (iter_type.has_method("current")) {
                    auto& current_method = iter_type.get_method("current");

                    if (var_type.lexeme.empty()) {
                        var_type.lexeme = current_method.return_type;
                        ctx.repeat_infer = true; // Repeat inference if type was inferred
                    }
                }
            }

            if (var_type.lexeme.empty()) {
                ctx.add_infer_error(var_name.line, var_name.column,
                    "Type for variable '" + var_name.lexeme + "' was not inferred, is necessary to annotate it.");
            }

            ctx.add_scope();

            ctx.add_variable(var_name.lexeme, var_type.lexeme);

            string body_type = body->infer(ctx, shouldbe_type);

            ctx.rollback_scope();

            if (body_type.empty()) {
                body_type = shouldbe_type; // If body type is not inferred, return the expected type
            }

            return body_type;
        }

        string binary_expr::infer(context& ctx, const string& shouldbe_type) {
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

        string unary_expr::infer(context& ctx, const string& shouldbe_type) {
            if (ast::unary_op::MINUS == op) {
                expression->infer(ctx, "Number");
                return "Number";
            }

            if (ast::unary_op::NOT == op) {
                expression->infer(ctx, "Boolean");
                return "Boolean";
            }

            return shouldbe_type; // If unary operator is not recognized, return the expected type
        }

        string literal_expr::infer(context& ctx, const string& shouldbe_type) {
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

        string var_expr::infer(context& ctx, const string& shouldbe_type) {
            if (object.has_value()) {
                string inferred_type = object.value()->infer(ctx);

                if (inferred_type.empty()) {
                    ctx.add_infer_error(name.line, name.column,
                        "Object type for variable '" + name.lexeme + "' could not be inferred.");
                    return shouldbe_type; // If object type is not inferred, return the expected type
                }

                auto& obj_type = ctx.get_type(inferred_type);

                if (!obj_type.has_field(name.lexeme)) {
                    ctx.add_infer_error(name.line, name.column,
                        "Object '" + inferred_type + "' does not have field '" + name.lexeme + "'.");
                    return shouldbe_type; // If field does not exist, return the expected type
                }

                auto& field = obj_type.get_field(name.lexeme);
                if (field.attr_type.empty()) {
                    field.attr_type = shouldbe_type;
                }

                return field.attr_type; // Return the type of the field
            }
            else {
                if (!ctx.variable_exists(name.lexeme)) {
                    ctx.add_infer_error(name.line, name.column,
                        "Variable '" + name.lexeme + "' does not exist.");
                    return shouldbe_type; // If variable does not exist, return the expected type
                }

                auto& var = ctx.get_variable(name.lexeme);

                if (var.attr_type.empty()) {
                    var.attr_type = shouldbe_type; // If variable type is not set, use the expected type
                }

                return var.attr_type;
            }
        }

        string new_expr::infer(context& ctx, const string& shouldbe_type) {
            auto& type = ctx.get_type(type_name.lexeme);

            for (int i = 0; i < arguments.size(); ++i) {
                arguments[i]->infer(ctx, type.params[i].attr_type);
            }

            return type_name.lexeme; // Return the type of the new expression
        }
    } // namespace ast
} // namespace hulk

#endif