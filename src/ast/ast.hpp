#ifndef HULK_AST_HPP
#define HULK_AST_HPP 1

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <string>

#include "enums"
#include "../internal/internal_uncopyable"
#include "../internal/internal_error"
#include "../semantic/context"

namespace hulk {
  namespace ast {
    static std::unique_ptr<llvm::LLVMContext> TheContext;
    static std::unique_ptr<llvm::Module> TheModule;
    static std::unique_ptr<llvm::IRBuilder<>> Builder;
    static std::map<std::string, llvm::AllocaInst*> NamedValues;
    static std::map<std::string, std::map<std::string, unsigned>> StructFieldIndices;
    static std::map<llvm::Value*, llvm::Type*> PointerType;

    // Expressions

    // Forwards declarations
    struct expr;
    struct binary_expr;
    struct call_expr;
    struct literal_expr;
    struct unary_expr;
    struct var_expr;
    struct assign_expr;
    struct declaration_expr;
    struct let_expr;
    struct while_expr;
    struct for_expr;
    struct if_expr;
    struct block_expr;
    struct new_expr;

    // Define smart pointers for expressions
    using expr_ptr = std::unique_ptr<expr>;
    using binary_expr_ptr = std::unique_ptr<binary_expr>;
    using call_expr_ptr = std::unique_ptr<call_expr>;
    using literal_expr_ptr = std::unique_ptr<literal_expr>;
    using unary_expr_ptr = std::unique_ptr<unary_expr>;
    using assign_expr_ptr = std::unique_ptr<assign_expr>;
    using declaration_expr_ptr = std::unique_ptr<declaration_expr>;
    using let_expr_ptr = std::unique_ptr<let_expr>;
    using while_expr_ptr = std::unique_ptr<while_expr>;
    using for_expr_ptr = std::unique_ptr<for_expr>;
    using var_expr_ptr = std::unique_ptr<var_expr>;
    using if_expr_ptr = std::unique_ptr<if_expr>;
    using block_expr_ptr = std::unique_ptr<block_expr>;
    using new_expr_ptr = std::unique_ptr<new_expr>;

    struct expr : private internal::uncopyable {
      virtual void scoped_visit(semantic::context& ctx) const {}
      virtual string infer(semantic::context& ctx, const string& shouldbe_type = "") { return ""; }
      virtual string type_check(semantic::context& ctx) { return "Object"; }

      virtual ~expr() = default;
      // This method is used to allow the visitor pattern
      virtual llvm::Value* codegen() { return nullptr; }
    };

    struct block_expr : public expr {
      std::vector<expr_ptr> expressions;

      explicit block_expr(std::vector<expr_ptr> _expressions)
        : expressions(std::move(_expressions)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      virtual llvm::Value* codegen() override;
    };

    struct binary_expr : public expr {
      expr_ptr left;
      lexer::token token;
      binary_op op;
      expr_ptr right;

      explicit binary_expr(expr_ptr _left, const lexer::token& _token,
        const binary_op _op, expr_ptr _right)
        : left(std::move(_left)),
        token(_token),
        op(_op),
        right(std::move(_right)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;

      llvm::Value* codegen() override;
    };

    struct unary_expr : public expr {
      lexer::token token;
      unary_op op;
      expr_ptr expression;

      explicit unary_expr(const lexer::token& _token, const unary_op _op,
        expr_ptr _expression)
        : token(_token), op(_op), expression(std::move(_expression)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;

      llvm::Value* codegen() override;
    };

    struct literal_expr : public expr {
      lexer::literal value;

      explicit literal_expr(const lexer::literal& _value) : value(_value) {}

      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;

      llvm::Value* codegen() override;
    };

    struct new_expr : public expr {
      lexer::token type_name;
      std::vector<expr_ptr> arguments;

      explicit new_expr(const lexer::token& _type_name,
        std::vector<expr_ptr> _arguments)
        : type_name(_type_name), arguments(std::move(_arguments)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct call_expr : public expr {
      std::optional<expr_ptr> object;
      lexer::token callee;
      std::vector<expr_ptr> arguments;

      explicit call_expr(std::optional<expr_ptr> _object, const lexer::token& _calle,
        std::vector<expr_ptr> _arguments)
        : object(std::move(_object)),
        callee(_calle),
        arguments(std::move(_arguments)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct let_expr : public expr {
      std::vector<declaration_expr_ptr> assignments;
      expr_ptr body;

      explicit let_expr(std::vector<declaration_expr_ptr> _assignments,
        expr_ptr _body)
        : assignments(std::move(_assignments)), body(std::move(_body)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct assign_expr : public expr {
      expr_ptr variable;
      lexer::token type;
      expr_ptr value;

      explicit assign_expr(expr_ptr _variable, const lexer::token& _type,
        expr_ptr _value)
        : variable(std::move(_variable)), type(_type), value(std::move(_value)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct declaration_expr : public expr {
      lexer::token name;
      lexer::token type;
      expr_ptr value;

      explicit declaration_expr(const lexer::token& _name,
        const lexer::token& _type, expr_ptr _value)
        : name(_name), type(_type), value(std::move(_value)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct if_expr : public expr {
      lexer::token condition_token; // Token for the 'if' keyword
      expr_ptr condition;
      expr_ptr then_branch;
      std::vector<lexer::token> elif_tokens;
      std::vector<std::pair<expr_ptr, expr_ptr>> elif_branchs;
      std::optional<expr_ptr> else_branch;

      explicit if_expr(lexer::token _condition_token, expr_ptr _condition, expr_ptr _then_branch,
        std::vector<lexer::token> elif_tokens, std::vector<std::pair<expr_ptr, expr_ptr>> _elif_branchs,
        std::optional<expr_ptr> _else_branch)
        : condition_token(_condition_token), 
        condition(std::move(_condition)),
        then_branch(std::move(_then_branch)),
        elif_tokens(std::move(elif_tokens)),
        elif_branchs(std::move(_elif_branchs)),
        else_branch(std::move(_else_branch)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct while_expr : public expr {
      lexer::token condition_token;
      expr_ptr condition;
      expr_ptr body;

      explicit while_expr(lexer::token _condition_token, expr_ptr _condition, expr_ptr _body)
        : condition_token(_condition_token), condition(std::move(_condition)), body(std::move(_body)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    struct for_expr : public expr {
      lexer::token var_name;
      lexer::token var_type;
      expr_ptr iterable;
      expr_ptr body;

      explicit for_expr(const lexer::token& _var_name,
        const lexer::token& _var_type, expr_ptr _iterable,
        expr_ptr _body)
        : var_name(_var_name),
        var_type(_var_type),
        iterable(std::move(_iterable)),
        body(std::move(_body)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
    };

    struct var_expr : public expr {
      std::optional<expr_ptr> object;
      lexer::token name;

      explicit var_expr(std::optional<expr_ptr> _object, const lexer::token& _name)
        : object(std::move(_object)), name(_name) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      string infer(semantic::context& ctx, const string& shouldbe_type = "") override;
      string type_check(semantic::context& ctx) override;
      llvm::Value* codegen() override;
    };

    // Statements

    // Forwards declarations
    struct stmt;
    struct expression_stmt;
    struct function_stmt;
    struct class_stmt;
    struct protocol_stmt;
    struct field_stmt;
    struct super_item;

    // Define smart pointers for statements
    using stmt_ptr = std::shared_ptr<stmt>;
    using expression_stmt_ptr = std::shared_ptr<expression_stmt>;
    using function_stmt_ptr = std::shared_ptr<function_stmt>;
    using class_stmt_ptr = std::shared_ptr<class_stmt>;
    using protocol_stmt_ptr = std::shared_ptr<protocol_stmt>;
    using field_stmt_ptr = std::shared_ptr<field_stmt>;
    using super_item_ptr = std::shared_ptr<super_item>;

    struct stmt : private internal::uncopyable {
      virtual void context_builder_visit(semantic::context& ctx) const {}
      virtual void scoped_visit(semantic::context& ctx) const {}
      virtual std::string infer(semantic::context& ctx, const std::string& shouldbe_type = "") { return ""; }
      virtual void type_check(semantic::context& ctx) const {}
      virtual llvm::Value* codegen() { return nullptr; }
    };

    struct parameter {
      lexer::token name;
      lexer::token type;

      explicit parameter(lexer::token _name, lexer::token _type)
        : name(_name), type(_type) {
      }
    };

    struct function_stmt : public stmt {
      lexer::token name;
      function_type type;
      std::vector<parameter> parameters;
      expr_ptr body;
      lexer::token return_type;

      explicit function_stmt(const lexer::token _name, const function_type _type,
        std::vector<parameter> _parameters, expr_ptr _body, const lexer::token _return_type)
        : name(_name), type(_type), parameters(std::move(_parameters)), body(std::move(_body)), return_type(_return_type) {
      }

      void context_builder_visit(semantic::context& ctx) const override;
      void scoped_visit(semantic::context& ctx) const override;
      std::string infer(semantic::context& ctx, const std::string& shouldbe_type = "") override;
      void type_check(semantic::context& ctx) const override;

      llvm::Function* codegen() override;
    };

    struct field_stmt : public stmt {
      lexer::token name;
      lexer::token type;
      expr_ptr initializer;

      explicit field_stmt(const lexer::token& _name, const lexer::token& _type,
        expr_ptr _initializer)
        : name(_name), type(_type), initializer(std::move(_initializer)) {
      }

      void scoped_visit(semantic::context& ctx) const override;
      std::string infer(semantic::context& ctx, const std::string& shouldbe_type = "") override;
      void type_check(semantic::context& ctx) const override;
    };

    struct super_item : private internal::uncopyable {
      lexer::token name;
      std::vector<expr_ptr> init;

      explicit super_item(const lexer::token& _name, std::vector<expr_ptr> _init)
        : name(_name), init(std::move(_init)) {
      }
    };

    struct class_stmt : public stmt {
      lexer::token name;
      std::vector<parameter> parameters;
      std::optional<super_item_ptr> super_class;
      std::vector<field_stmt_ptr> fields;
      std::vector<function_stmt_ptr> methods;

      explicit class_stmt(const lexer::token& _name, std::vector<parameter> _parameters, std::optional<super_item_ptr> _super,
        std::vector<field_stmt_ptr> _fields, std::vector<function_stmt_ptr> _methods)
        : name(_name), super_class(std::move(_super)), parameters(std::move(_parameters)),
        fields(std::move(_fields)), methods(std::move(_methods)) {
      }

      void context_builder_visit(semantic::context& ctx) const override;
      void scoped_visit(semantic::context& ctx) const override;
      std::string infer(semantic::context& ctx, const std::string& shouldbe_type = "") override;
      void type_check(semantic::context& ctx) const override;

      llvm::Value* codegen() override;
    };

    struct protocol_stmt : public stmt {
      lexer::token name;
      std::vector<function_stmt_ptr> methods;
      lexer::token super_protocol;

      explicit protocol_stmt(const lexer::token& _name,
        std::vector<function_stmt_ptr> _methods,
        lexer::token _super_protocol)
        : name(_name),
        methods(std::move(_methods)),
        super_protocol(_super_protocol) {
      }

      void context_builder_visit(semantic::context& ctx) const override;
    };

    struct expression_stmt : public stmt {
      expr_ptr expression;
      explicit expression_stmt(expr_ptr _expression)
        : expression(std::move(_expression)) {
      }
    };

    // Program
    struct program : private internal::uncopyable {
      std::vector<stmt_ptr> statements;
      expr_ptr main;

      explicit program(std::vector<stmt_ptr> _statements, expr_ptr _main)
        : statements(std::move(_statements)), main(std::move(_main)) {
      }

      void context_builder_visit(semantic::context& ctx) const;
      void scoped_visit(semantic::context& ctx) const;
      std::string infer(semantic::context& ctx, const std::string& shouldbe_type = "") const;
      void type_check(semantic::context& ctx) const;

      llvm::Value* codegen() const;
    };

  }  // namespace ast

}  // namespace hulk

#endif  // HULK_AST_HPP