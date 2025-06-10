#ifndef HULK_AST_NODE_HPP
#define HULK_AST_NODE_HPP

#include <vector>
#include <string>
#include "../semantic/context.hpp"
using namespace std;


namespace hulk {
    using type = semantic::type;
    namespace ast {
        // Forward declarations
        struct statement;
        struct expression;

        // Abstract Syntax Tree (AST) Node Base Class
        struct node {
            virtual void context_builder_visit(semantic::context& ctx) const {}
            virtual void scoped_visit(semantic::context& ctx) const {}
            virtual string infer(semantic::context& ctx, const string& shouldbe_type = "") {}
            virtual string print() const { return "node"; }
        };

        struct program : node {
            vector<statement*> statements;
            expression* main;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            void type_check(semantic::context& ctx) const;
        };

        // Statements
        struct statement : node {
            virtual void type_check(semantic::context& ctx) const {}
        };

        struct param {
            string id;
            string type;
        };

        struct def_field : statement {
            string id;
            string var_type;
            expression* value;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            void type_check(semantic::context& ctx) const override;
        };

        struct def_function : statement {
            string id;
            vector<param> params;
            string return_type;
            expression* body;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            void type_check(semantic::context& ctx) const override;
        };

        struct def_type : statement {
            string id;
            vector<param> params;
            string parent;
            vector<expression*> parent_args;
            vector<def_field*> fields;
            vector<def_function*> methods;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            void type_check(semantic::context& ctx) const override;
        };

        // Expressions
        struct expression : node {
            type* return_type;

            virtual type* type_check(semantic::context& ctx) { return nullptr; }
        };

        struct block_expr : expression {
            vector<expression*> exprs;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct call_expr : expression {
            string id;
            vector<expression*> args;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct let_expr : expression {
            string id;
            string var_type;
            expression* value;
            expression* body;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct assign_expr : expression {
            string id;
            expression* value;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct if_expr : expression {
            expression* condition;
            expression* then_branch;
            expression* else_branch; // Optional

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct while_expr : expression {
            expression* condition;
            expression* body;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        /* Arithmetic expressions */

        struct binary_expr : expression {
            string op;
            expression* left;
            expression* right;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct unary_expr : expression {
            string op;
            expression* expr;

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        /* Literals */
        struct variable : expression {
            string id;

            variable(const string& id) : id(id) {}

            void scoped_visit(semantic::context& ctx) const override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
            type* type_check(semantic::context& ctx) override;
        };

        struct number : expression {
            string value;

            number(const string& value) : value(value) {}

            type* type_check(semantic::context& ctx) override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
        };

        struct boolean : expression {
            string value;

            boolean(const string& value) : value(value) {}

            type* type_check(semantic::context& ctx) override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
        };

        struct string_ : expression {
            string value;

            string_(const string& value) : value(value) {}

            type* type_check(semantic::context& ctx) override;
            string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
        };

        struct literal : expression { // this probably won't be used
            string value; // "number", "boolean", "string"
            string type;

            literal(string value, string type) : value(value), type(type) {}
        };

    }   // namespace ast
} // namespace hulk

#endif // HULK_AST_NODE_HPP