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
            virtual void visit(semantic::context& ctx) const;

            virtual string print() const {
                return "node";
            }
        };

        struct program : node {
            vector<statement*> statements;
            expression* main;

            void visit(semantic::context& ctx) const override;
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

            void visit(semantic::context& ctx) const override;
        };

        struct def_function : statement {
            string id;
            vector<param> params;
            string return_type;
            expression* body;

            void visit(semantic::context& ctx) const override;
        };

        struct def_field : statement {
            string id;
            string type;
            expression* value;
        };

        // Expressions

        struct expression : node {};

        struct block_expr : expression {
            vector<expression*> exprs;
        };

        struct call_expr : expression {
            string id;
            vector<expression*> args;
        };

        struct let_expr : expression {
            string id;
            string type;
            expression* value;
            expression* body;
        };

        struct assign_expr : expression {
            string id;
            expression* value;
        };

        struct if_expr : expression {
            expression* condition;
            expression* then_branch;
            expression* else_branch; // Optional
        };

        struct while_expr : expression {
            expression* condition;
            expression* body;
        };

        struct with_expr : expression {
            expression* expr;
            string id;
            expression* then_branch;
            expression* else_branch; // Optional 
        };

        /* Arithmetic expressions */

        struct binary_expr : expression {
            string op;
            expression* left;
            expression* right;
        };

        struct unary_expr : expression {
            string op;
            expression* expr;
        };

        /* Literals */

        struct literal : expression {
            string value; // "number", "boolean", "string"
        };

        struct number : literal {
            double value;
        };

        struct boolean : literal {
            bool value;
        };

        struct string_expr : literal {
            string value;
        };

        /* Builtin functions */

        struct builtin_function : expression {
            string name;
            vector<expression*> args;
        };

    }   // namespace ast
} // namespace hulk

#endif // HULK_AST_NODE_HPP