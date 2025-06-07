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
        struct param;
        struct assign_expr;
        struct def_function;
        struct def_type;
        struct def_field;

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
        };

        // Statements

        struct statement : node {};

        struct param {
            string id;
            string type;
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

        struct def_function : statement {
            string id;
            vector<param> params;
            string return_type;
            expression* body;

            void context_builder_visit(semantic::context& ctx) const override;
            void scoped_visit(semantic::context& ctx) const override;
        };

        struct def_field : statement {
            string id;
            string type;
            expression* value;

            void scoped_visit(semantic::context& ctx) const override;
        };

        // Expressions

        struct expression : node {};

        struct block_expr : expression {
            vector<expression*> exprs;

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct call_expr : expression {
            string id;
            vector<expression*> args;

            void scoped_visit(semantic::context& ctx) const override;
        };

        struct let_expr : expression {
            string id;
            string type;
            expression* value;
            expression* body;

            void scoped_visit(semantic::context& ctx) const override;
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
        struct literal : expression {
            string value; // "number", "boolean", "string"
            string type;

            literal(string value, string type) : value(value), type(type) {}
        };

    }   // namespace ast
} // namespace hulk

#endif // HULK_AST_NODE_HPP