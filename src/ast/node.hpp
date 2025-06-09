#ifndef HULK_AST_NODE_HPP
#define HULK_AST_NODE_HPP

#include <vector>
#include <string>
#include "../semantic/context.hpp"
using namespace std;

namespace hulk {
    namespace ast {
        // Forward declarations
        struct statement;
        struct expression;

        // Abstract Syntax Tree (AST) Node Base Class
        struct node {
            virtual void context_builder_visit(semantic::context& ctx) const {}
            virtual void scoped_visit(semantic::context& ctx) const {}
            virtual string print() const { return "node"; }
        };

        struct program : node {
            vector<statement*> statements;
            expression* main;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
            // void infer(semantic::context& ctx) const;
            // void type_check(semantic::context& ctx) const;
        };

        // Statements

        struct statement : node {
            virtual void infer(semantic::context& ctx) {}
            virtual void type_check(semantic::context& ctx) const {}
        };

        struct param {
            string id;
            string type;
        };

        struct def_field : statement {
            string id;
            string type;
            expression* value;

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct def_function : statement {
            string id;
            vector<param> params;
            string return_type;
            expression* body;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
        };

        struct def_type : statement {
            string id;
            vector<param> params;
            vector<string> parents;
            vector<def_field*> fields;
            vector<def_function*> methods;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
        };


        // Expressions

        struct expression : node {
            string return_type;
            virtual string infer(semantic::context& ctx, const string& type_name = "") { return ""; }
            virtual string type_check(semantic::context& ctx) const { return ""; }
        };

        struct block_expr : expression {
            vector<expression*> exprs;

            void scoped_visit(semantic::context& ctx) const override;
            // string type_check(semantic::context& ctx) const override;
        };

        struct call_expr : expression {
            string id;
            vector<expression*> args;

            void scoped_visit(semantic::context& ctx) const override;
            // string type_check(semantic::context& ctx) const override;
        };

        struct let_expr : expression {
            string id;
            string type;
            expression* value;
            expression* body;

            void scoped_visit(semantic::context& ctx) const override;
            // string type_check(semantic::context& ctx) const override;
        };

        struct assign_expr : expression {
            string id;
            expression* value;

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct if_expr : expression {
            expression* condition;
            expression* then_branch;
            expression* else_branch; // Optional

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct while_expr : expression {
            expression* condition;
            expression* body;

            void scoped_visit(semantic::context& ctx) const override;
        };

        /* Arithmetic expressions */

        struct binary_expr : expression {
            string op;
            expression* left;
            expression* right;

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct unary_expr : expression {
            string op;
            expression* expr;

            void scoped_visit(semantic::context& ctx) const override;
        };

        /* Literals */

        struct variable : expression {
            string id;

            variable(const string& id) : id(id) {}

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct number : expression {
            string value;
            number(const string& value) : value(value) {}
        };

        struct boolean : expression {
            string value;
            boolean(const string& value) : value(value) {}
        };

        struct string_ : expression {
            string value;
            string_(const string& value) : value(value) {}
        };

        struct literal : expression { // this probably won't be used
            string value; // "number", "boolean", "string"
            string type;

            literal(string value, string type) : value(value), type(type) {}
        };

    }   // namespace ast
} // namespace hulk

#endif // HULK_AST_NODE_HPP