#ifndef HULK_AST_BUILDER_HPP
#define HULK_AST_BUILDER_HPP

#include "../ast/ast"
#include "gen_parser.hpp"
#include "grammar_builder.hpp"

using namespace std;
using namespace hulk::ast;
using namespace hulk::lexer;

namespace hulk {

namespace gen_parser {

struct ast_builder {
  program build(derivation_node* root) {
    assert(root->symbol == "program");
    return visit_program(root);
  }

 private:
  int cnt_stmt = 0;

  // program -> declaration_list
  program visit_program(derivation_node* node) {
    vector<stmt_ptr> stmts = visit_declaration_list(node->children[0].get());
    expr_ptr main_expr;
    auto last = stmts.back();
    if (ast::expression_stmt* t =
            dynamic_cast<ast::expression_stmt*>(last.get())) {
      main_expr = std::move(t->expression);
      stmts.pop_back();
    }
    return ast::program(std::move(stmts), std::move(main_expr));
  }

  // declaration_list -> declaration sep_semicolon declaration_list | ε
  std::vector<stmt_ptr> visit_declaration_list(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<stmt_ptr> result;
    result.push_back(visit_declaration(node->children[0].get()));

    if (node->children.size() > 2) {  // Has more declarations
      auto decls = visit_declaration_list(node->children[2].get());
      result.insert(result.end(), std::make_move_iterator(decls.begin()),
                    std::make_move_iterator(decls.end()));
    }

    return result;
  }

  // declaration -> function_decl | type_decl | protocol_decl | stmt
  stmt_ptr visit_declaration(derivation_node* node) {
    auto child = node->children[0].get();
    if (child->symbol == "function_decl") {
      return visit_function_decl(child);
    } else if (child->symbol == "type_decl") {
      return visit_type_decl(child);
    } else if (child->symbol == "protocol_decl") {
      return visit_protocol_decl(child);
    } else {
      cnt_stmt++;

      if (cnt_stmt > 1) {
        error(*child->node_token, "A program in HULK can consist of just one global expression.");
        return nullptr;
      }

      return visit_statement(child);
    }
  }

  // type_decl -> KW_TYPE IDENTIFIER type_params type_inheritance LBRACE
  // type_body RBRACE
  stmt_ptr visit_type_decl(derivation_node* node) {
    token name = *node->children[1]->node_token;
    vector<parameter> params = visit_type_params(node->children[2].get());
    optional<super_item_ptr> super =
        visit_type_inheritance(node->children[3].get());

    vector<field_stmt_ptr> fields;
    vector<function_stmt_ptr> methods;
    visit_type_body(node->children[5].get(), fields, methods);

    return make_shared<class_stmt>(name, std::move(params), std::move(super),
                                   std::move(fields), std::move(methods));
  }

  // type_params -> LPAREN t_param_list RPAREN | ε
  vector<parameter> visit_type_params(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};
    return visit_t_param_list(node->children[1].get());
  }

  // t_param_list -> t_param t_param_list_tail
  vector<parameter> visit_t_param_list(derivation_node* node) {
    vector<parameter> params;
    params.push_back(visit_t_param(node->children[0].get()));

    if (node->children.size() > 1) {
      auto more = visit_t_param_list_tail(node->children[1].get());
      params.insert(params.end(), std::make_move_iterator(more.begin()),
                    std::make_move_iterator(more.end()));
    }

    return params;
  }

  // t_param -> IDENTIFIER opt_type_annotation
  parameter visit_t_param(derivation_node* node) {
    token name = *node->children[0]->node_token;
    token type;

    if (node->children.size() > 1 &&
        node->children[1]->children[0]->symbol != epsilon) {
      type = *node->children[1]->children[1]->node_token;
    }

    return parameter(name, type);
  }

  // t_param_list_tail -> COMMA t_param t_param_list_tail | ε
  vector<parameter> visit_t_param_list_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<parameter> params;
    params.push_back(visit_t_param(node->children[1].get()));

    if (node->children.size() > 2) {
      auto more = visit_t_param_list_tail(node->children[2].get());
      params.insert(params.end(), std::make_move_iterator(more.begin()),
                    std::make_move_iterator(more.end()));
    }

    return params;
  }

  // type_inheritance -> KW_INHERITS IDENTIFIER parent_args | ε
  optional<super_item_ptr> visit_type_inheritance(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return nullopt;

    token name = *node->children[1]->node_token;
    vector<expr_ptr> args = visit_parent_args(node->children[2].get());

    return make_unique<super_item>(name, std::move(args));
  }

  // parent_args -> LPAREN args RPAREN | ε
  vector<expr_ptr> visit_parent_args(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};
    return visit_args(node->children[1].get());
  }

  // type_body -> member type_body | ε
  void visit_type_body(derivation_node* node, vector<field_stmt_ptr>& fields,
                       vector<function_stmt_ptr>& methods) {
    if (node->children[0]->symbol == epsilon) return;

    auto member = node->children[0].get();
    if (member->children[1]->symbol == "member_tail" &&
        member->children[1]->children[0]->symbol != "LPAREN") {
      // Field declaration
      token name = *member->children[0]->node_token;
      token type;
      expr_ptr init;

      auto tail = member->children[1].get();
      if (tail->children[0]->children[0]->symbol !=
          epsilon) {  // Has type annotation
        type = *tail->children[0]->children[1]->node_token;
      }

      if (tail->children[1]->children[0]->symbol !=
          epsilon) {  // Has initializer
        auto expr = tail->children[1]->children[0]->children[1].get();
        init = visit_expression(expr);
      }

      fields.push_back(make_shared<field_stmt>(name, type, std::move(init)));
    } else {
      // Method declaration
      methods.push_back(visit_method(member));
    }

    visit_type_body(node->children[1].get(), fields, methods);
  }

  // member -> IDENTIFIER member_tail
  // member_tail -> LPAREN args RPAREN opt_type_annotation funtion_body
  // sep_semicolon
  function_stmt_ptr visit_method(derivation_node* node) {
    token name = *node->children[0]->node_token;
    auto tail = node->children[1].get();

    vector<parameter> params = visit_opt_param_list(tail->children[1].get());
    token return_type;

    if (tail->children[3]->children[0]->symbol != epsilon) {  // Has return type
      return_type = *tail->children[3]->children[1]->node_token;
    }

    expr_ptr body = visit_function_body(tail->children[4].get());

    return make_shared<function_stmt>(name, function_type::METHOD,
                                      std::move(params), std::move(body),
                                      return_type);
  }

  // method_params -> m_param_list | ε
  vector<parameter> visit_method_params(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};
    return visit_m_param_list(node->children[0].get());
  }

  // m_param_list -> m_param m_param_list_tail
  vector<parameter> visit_m_param_list(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<parameter> params;
    params.push_back(visit_m_param(node->children[0].get()));

    if (node->children.size() > 1) {
      auto more = visit_m_param_list_tail(node->children[1].get());
      params.insert(params.end(), make_move_iterator(more.begin()),
                    make_move_iterator(more.end()));
    }

    return params;
  }

  // m_param -> IDENTIFIER type_annotation
  parameter visit_m_param(derivation_node* node) {
    token name = *node->children[0]->node_token;
    token type;

    if (node->children[1]->children[0]->symbol != epsilon)
      type = *node->children[1]->children[1]->node_token;

    return parameter(name, type);
  }

  // m_param_list_tail -> COMMA m_param m_param_list_tail | ε
  vector<parameter> visit_m_param_list_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<parameter> params;
    params.push_back(visit_m_param(node->children[1].get()));

    if (node->children.size() > 2) {
      auto more = visit_m_param_list_tail(node->children[2].get());
      params.insert(params.end(), std::make_move_iterator(more.begin()),
                    std::make_move_iterator(more.end()));
    }

    return params;
  }

  // protocol_decl -> KW_PROTOCOL IDENTIFIER protocol_inheritance LBRACE
  // protocol_body RBRACE
  stmt_ptr visit_protocol_decl(derivation_node* node) {
    token name = *node->children[1]->node_token;
    token super;

    if (node->children[2]->children[0]->symbol != epsilon) {
      super = *node->children[2]->children[1]->node_token;
    }

    vector<function_stmt_ptr> methods =
        visit_protocol_body(node->children[4].get());
    return make_shared<protocol_stmt>(name, std::move(methods), super);
  }

  // protocol_body -> protocol_method protocol_body_tail | ε
  vector<function_stmt_ptr> visit_protocol_body(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<function_stmt_ptr> methods;
    methods.push_back(visit_protocol_method(node->children[0].get()));

    if (node->children.size() > 1) {
      auto more = visit_protocol_body_tail(node->children[1].get());
      methods.insert(methods.end(), make_move_iterator(more.begin()),
                     make_move_iterator(more.end()));
    }

    return methods;
  }

  // protocol_method -> IDENTIFIER LPAREN method_params RPAREN type_annotation
  // SEMICOLON
  function_stmt_ptr visit_protocol_method(derivation_node* node) {
    token name = *node->children[0]->node_token;
    vector<parameter> params = visit_method_params(node->children[2].get());
    token return_type = *node->children[4]->children[1]->node_token;

    return make_shared<function_stmt>(name, function_type::METHOD,
                                      std::move(params), nullptr, return_type);
  }

  // protocol_body_tail -> SEMICOLON protocol_method protocol_body_tail | ε
  vector<function_stmt_ptr> visit_protocol_body_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<function_stmt_ptr> methods;
    methods.push_back(visit_protocol_method(node->children[1].get()));

    if (node->children.size() > 2) {
      auto more = visit_protocol_body_tail(node->children[2].get());
      methods.insert(methods.end(), make_move_iterator(more.begin()),
                     make_move_iterator(more.end()));
    }

    return methods;
  }

  // function_decl -> KW_FUNCTION IDENTIFIER LPAREN opt_param_list RPAREN
  // opt_type_annotation function_body
  stmt_ptr visit_function_decl(derivation_node* node) {
    token name = *node->children[1]->node_token;
    vector<parameter> params = visit_opt_param_list(node->children[3].get());

    token return_type;
    if (node->children[5]->children[0]->symbol != epsilon)
      return_type = *node->children[5]->children[1]->node_token;

    expr_ptr body = visit_function_body(node->children[6].get());

    return make_shared<function_stmt>(name, function_type::FUNCTION,
                                      std::move(params), std::move(body),
                                      return_type);
  }

  // opt_param_list -> param_list | ε
  vector<parameter> visit_opt_param_list(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};
    return visit_param_list(node->children[0].get());
  }

  // param_list -> param param_list_tail | ε
  vector<parameter> visit_param_list(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<parameter> params;
    params.push_back(visit_param(node->children[0].get()));

    if (node->children.size() > 1) {
      auto more_params = visit_param_list_tail(node->children[1].get());
      params.insert(params.end(), make_move_iterator(more_params.begin()),
                    make_move_iterator(more_params.end()));
    }

    return params;
  }

  // param -> IDENTIFIER opt_type_annotation
  parameter visit_param(derivation_node* node) {
    token name = *node->children[0]->node_token;
    token type;

    if (node->children.size() > 1 &&
        node->children[1]->children[0]->symbol != epsilon) {
      type = *node->children[1]->children[1]->node_token;
    }

    return parameter(name, type);
  }

  // param_list_tail -> COMMA param param_list_tail | ε
  vector<parameter> visit_param_list_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<parameter> params;
    params.push_back(visit_param(node->children[1].get()));

    if (node->children.size() > 2) {  // Has more params
      auto more_params = visit_param_list_tail(node->children[2].get());
      params.insert(params.end(), make_move_iterator(more_params.begin()),
                    make_move_iterator(more_params.end()));
    }

    return params;
  }

  // function_body -> ARROW expression | block_expr
  expr_ptr visit_function_body(derivation_node* node) {
    if (node->children[0]->symbol == "ARROW") {
      return visit_expression(node->children[1].get());
    } else {
      return visit_block_expr(node->children[0].get());
    }
  }

  // stmt -> expression_statement
  stmt_ptr visit_statement(derivation_node* node) {
    return make_shared<expression_stmt>(
        visit_expression_statement(node->children[0].get()));
  }

  // expression_statement -> expression
  expr_ptr visit_expression_statement(derivation_node* node) {
    return visit_expression(node->children[0].get());
  }

  // expression -> let_expr | if_expr | while_expr | block_expr | for_expr |
  // assignment
  expr_ptr visit_expression(derivation_node* node) {
    auto child = node->children[0].get();

    if (child->symbol == "let_expr") return visit_let_expr(child);

    if (child->symbol == "if_expr") return visit_if_expr(child);

    if (child->symbol == "while_expr") return visit_while_expr(child);

    if (child->symbol == "block_expr") return visit_block_expr(child);

    if (child->symbol == "for_expr") return visit_for_expr(child);

    return visit_assignment(child);
  }

  // while_expr -> KW_WHILE LPAREN expression RPAREN expression
  expr_ptr visit_while_expr(derivation_node* node) {
    token while_token = *node->children[0]->node_token;
    expr_ptr condition = visit_expression(node->children[2].get());
    expr_ptr body = visit_expression(node->children[4].get());
    return make_unique<while_expr>(while_token, std::move(condition),
                                   std::move(body));
  }

  // if_expr -> KW_IF LPAREN expression RPAREN expression elif_list KW_ELSE
  // expression
  expr_ptr visit_if_expr(derivation_node* node) {
    token if_token = *node->children[0]->node_token;
    expr_ptr condition = visit_expression(node->children[2].get());
    expr_ptr then_branch = visit_expression(node->children[4].get());

    vector<token> elif_tokens;
    vector<pair<expr_ptr, expr_ptr>> elif_branches;
    visit_elif_list(node->children[5].get(), elif_tokens, elif_branches);

    expr_ptr else_branch = visit_expression(node->children[7].get());

    return make_unique<if_expr>(if_token, std::move(condition),
                                std::move(then_branch), std::move(elif_tokens),
                                std::move(elif_branches),
                                std::move(else_branch));
  }

  // elif_list -> KW_ELIF LPAREN expression RPAREN expression elif_list | ε
  void visit_elif_list(derivation_node* node, vector<token>& tokens,
                       vector<pair<expr_ptr, expr_ptr>>& branches) {
    if (node->children[0]->symbol == epsilon) return;

    tokens.push_back(*node->children[0]->node_token);
    expr_ptr condition = visit_expression(node->children[2].get());
    expr_ptr then_branch = visit_expression(node->children[4].get());
    branches.emplace_back(std::move(condition), std::move(then_branch));

    visit_elif_list(node->children[5].get(), tokens, branches);
  }

  // let_expr -> KW_LET binding_list KW_IN expression
  expr_ptr visit_let_expr(derivation_node* node) {
    vector<declaration_expr_ptr> bindings =
        visit_binding_list(node->children[1].get());
    expr_ptr body = visit_expression(node->children[3].get());
    return make_unique<let_expr>(std::move(bindings), std::move(body));
  }

  // binding_list -> binding binding_list_tail
  vector<declaration_expr_ptr> visit_binding_list(derivation_node* node) {
    vector<declaration_expr_ptr> bindings;
    bindings.push_back(visit_binding(node->children[0].get()));

    if (node->children.size() > 1) {
      auto more = visit_binding_list_tail(node->children[1].get());
      bindings.insert(bindings.end(), make_move_iterator(more.begin()),
                      make_move_iterator(more.end()));
    }

    return bindings;
  }

  // binding -> IDENTIFIER opt_type_annotation initializer
  declaration_expr_ptr visit_binding(derivation_node* node) {
    token name = *node->children[0]->node_token;
    token type;

    if (node->children[1]->children[0]->symbol != epsilon) {
      type = *node->children[1]->children[1]->node_token;
    }

    expr_ptr init = visit_initializer(node->children[2].get());
    return make_unique<declaration_expr>(name, type, std::move(init));
  }

  // initializer -> OP_ASSIGN expression
  expr_ptr visit_initializer(derivation_node* node) {
    return visit_expression(node->children[1].get());
  }

  // binding_list_tail -> COMMA binding binding_list_tail | ε
  vector<declaration_expr_ptr> visit_binding_list_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<declaration_expr_ptr> bindings;
    bindings.push_back(visit_binding(node->children[1].get()));

    if (node->children.size() > 2) {
      auto more = visit_binding_list_tail(node->children[2].get());
      bindings.insert(bindings.end(), std::make_move_iterator(more.begin()),
                      std::make_move_iterator(more.end()));
    }

    return bindings;
  }

  // block_expr -> LBRACE expression_list RBRACE
  expr_ptr visit_block_expr(derivation_node* node) {
    vector<expr_ptr> exprs = visit_expression_list(node->children[1].get());
    return make_unique<block_expr>(std::move(exprs));
  }

  // expression_list -> opt_expression expression_list_tail
  vector<expr_ptr> visit_expression_list(derivation_node* node) {
    vector<expr_ptr> exprs;

    if (node->children[0]->children[0]->symbol != epsilon) {
      exprs.push_back(visit_expression(node->children[0]->children[0].get()));
    }

    if (node->children.size() > 1) {
      auto more = visit_expression_list_tail(node->children[1].get());
      exprs.insert(exprs.end(), make_move_iterator(more.begin()),
                   make_move_iterator(more.end()));
    }

    return exprs;
  }

  // expression_list_tail -> SEMICOLON opt_expression expression_list_tail | ε
  vector<expr_ptr> visit_expression_list_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<expr_ptr> exprs;

    if (node->children[1]->children[0]->symbol != epsilon) {
      exprs.push_back(visit_expression(node->children[1]->children[0].get()));
    }

    if (node->children.size() > 2) {
      auto more = visit_expression_list_tail(node->children[2].get());
      exprs.insert(exprs.end(), make_move_iterator(more.begin()),
                   make_move_iterator(more.end()));
    }

    return exprs;
  }

  // for_expr -> KW_FOR LPAREN IDENTIFIER opt_type_annotation KW_IN expression
  // RPAREN expression
  expr_ptr visit_for_expr(derivation_node* node) {
    token var_name = *node->children[2]->node_token;
    token var_type;

    if (node->children[3]->children[0]->symbol != epsilon) {
      var_type = *node->children[3]->children[1]->node_token;
    }

    expr_ptr iterable = visit_expression(node->children[5].get());
    expr_ptr body = visit_expression(node->children[7].get());

    return make_unique<for_expr>(var_name, var_type, std::move(iterable),
                                 std::move(body));
  }

  // assignment -> or_expr assignment_tail
  expr_ptr visit_assignment(derivation_node* node) {
    expr_ptr left = visit_or_expr(node->children[0].get());
    return visit_assignment_tail(node->children[1].get(), std::move(left));
  }

  // assignment_tail -> OP_DESTRUCT_ASSIGN or_expr assignment_tail | ε
  expr_ptr visit_assignment_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    expr_ptr right = visit_or_expr(node->children[1].get());

    auto assign =
        make_unique<assign_expr>(std::move(left), op, std::move(right));

    if (node->children.size() > 2)
      return visit_assignment_tail(node->children[2].get(), std::move(assign));

    return assign;
  }

  // or_expr -> and_expr or_expr_tail
  expr_ptr visit_or_expr(derivation_node* node) {
    expr_ptr left = visit_and_expr(node->children[0].get());
    return visit_or_expr_tail(node->children[1].get(), std::move(left));
  }

  // or_expr_tail -> OP_OR and_expr or_expr_tail | ε
  expr_ptr visit_or_expr_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    expr_ptr right = visit_and_expr(node->children[1].get());

    auto binary = make_unique<binary_expr>(std::move(left), op, binary_op::OR,
                                           std::move(right));

    if (node->children.size() > 2) {
      return visit_or_expr_tail(node->children[2].get(), std::move(binary));
    }

    return binary;
  }

  // and_expr -> equality and_expr_tail
  expr_ptr visit_and_expr(derivation_node* node) {
    expr_ptr left = visit_equality(node->children[0].get());
    return visit_and_expr_tail(node->children[1].get(), std::move(left));
  }

  // and_expr_tail -> OP_AND equality and_expr_tail | ε
  expr_ptr visit_and_expr_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    expr_ptr right = visit_equality(node->children[1].get());

    auto binary = make_unique<binary_expr>(std::move(left), op, binary_op::AND,
                                           std::move(right));

    if (node->children.size() > 2) {
      return visit_and_expr_tail(node->children[2].get(), std::move(binary));
    }

    return binary;
  }

  // equality -> comparison equality_tail
  expr_ptr visit_equality(derivation_node* node) {
    expr_ptr left = visit_comparison(node->children[0].get());
    return visit_equality_tail(node->children[1].get(), std::move(left));
  }

  // equality_tail -> OP_NOT_EQUAL comparison equality_tail | OP_EQUAL
  // comparison equality_tail | ε
  expr_ptr visit_equality_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    binary_op bop = op.type == token_type::OP_EQUAL ? binary_op::EQUAL_EQUAL
                                                    : binary_op::NOT_EQUAL;
    expr_ptr right = visit_comparison(node->children[1].get());

    auto binary =
        make_unique<binary_expr>(std::move(left), op, bop, std::move(right));

    if (node->children.size() > 2) {
      return visit_equality_tail(node->children[2].get(), std::move(binary));
    }

    return binary;
  }

  // comparison -> term comparison_tail
  expr_ptr visit_comparison(derivation_node* node) {
    expr_ptr left = visit_term(node->children[0].get());
    return visit_comparison_tail(node->children[1].get(), std::move(left));
  }

  // comparison_tail -> OP_GREATER term comparison_tail | OP_GREATER_EQ term
  // comparison_tail |
  //                    OP_LESS term comparison_tail | OP_LESS_EQ term
  //                    comparison_tail | ε
  expr_ptr visit_comparison_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    binary_op bop;

    switch (op.type) {
      case token_type::OP_GREATER:
        bop = binary_op::GREATER;
        break;
      case token_type::OP_GREATER_EQ:
        bop = binary_op::GREATER_EQUAL;
        break;
      case token_type::OP_LESS:
        bop = binary_op::LESS;
        break;
      case token_type::OP_LESS_EQ:
        bop = binary_op::LESS_EQUAL;
        break;
      default:
        assert(false && "Invalid comparison operator");
    }

    expr_ptr right = visit_term(node->children[1].get());
    auto binary =
        make_unique<binary_expr>(std::move(left), op, bop, std::move(right));

    if (node->children.size() > 2) {
      return visit_comparison_tail(node->children[2].get(), std::move(binary));
    }

    return binary;
  }

  // term -> factor term_tail
  expr_ptr visit_term(derivation_node* node) {
    expr_ptr left = visit_factor(node->children[0].get());
    return visit_term_tail(node->children[1].get(), std::move(left));
  }

  // term_tail -> OP_MINUS factor term_tail | OP_PLUS factor term_tail | ε
  expr_ptr visit_term_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    binary_op bop =
        op.type == token_type::OP_PLUS ? binary_op::PLUS : binary_op::MINUS;
    expr_ptr right = visit_factor(node->children[1].get());

    auto binary =
        make_unique<binary_expr>(std::move(left), op, bop, std::move(right));

    if (node->children.size() > 2) {
      return visit_term_tail(node->children[2].get(), std::move(binary));
    }

    return binary;
  }

  // factor -> unary factor_tail
  expr_ptr visit_factor(derivation_node* node) {
    expr_ptr left = visit_unary(node->children[0].get());
    return visit_factor_tail(node->children[1].get(), std::move(left));
  }

  // factor_tail -> OP_DIVIDE unary factor_tail | OP_MODULE unary factor_tail |
  //                OP_MULTIPLY unary factor_tail | OP_EXPONENT unary
  //                factor_tail | OP_CONCAT unary factor_tail | OP_DOBLE_CONCAT
  //                unary factor_tail | ε
  expr_ptr visit_factor_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    token op = *node->children[0]->node_token;
    binary_op bop;

    switch (op.type) {
      case token_type::OP_DIVIDE:
        bop = binary_op::DIVIDE;
        break;
      case token_type::OP_MODULE:
        bop = binary_op::MODULE;
        break;
      case token_type::OP_MULTIPLY:
        bop = binary_op::MULT;
        break;
      case token_type::OP_EXPONENT:
        bop = binary_op::EXPONENT;
        break;
      case token_type::OP_CONCAT:
        bop = binary_op::CONCAT;
        break;
      case token_type::OP_DOBLE_CONCAT:
        bop = binary_op::CONCAT_DOBLE;
        break;
      default:
        assert(false && "Invalid factor operator");
    }

    expr_ptr right = visit_unary(node->children[1].get());
    auto binary =
        make_unique<binary_expr>(std::move(left), op, bop, std::move(right));

    if (node->children.size() > 2) {
      return visit_factor_tail(node->children[2].get(), std::move(binary));
    }

    return binary;
  }

  // unary -> OP_NOT unary | OP_MINUS unary | var_call
  expr_ptr visit_unary(derivation_node* node) {
    if (node->children[0]->symbol == "var_call") {
      return visit_var_call(node->children[0].get());
    }

    token op = *node->children[0]->node_token;
    unary_op uop =
        op.type == token_type::OP_NOT ? unary_op::NOT : unary_op::MINUS;
    expr_ptr right = visit_unary(node->children[1].get());

    return make_unique<unary_expr>(op, uop, std::move(right));
  }

  // var_call -> primary var_call_tail
  expr_ptr visit_var_call(derivation_node* node) {
    expr_ptr left = visit_primary(node->children[0].get());
    return visit_var_call_tail(node->children[1].get(), std::move(left));
  }

  // var_call_tail -> DOT IDENTIFIER finish_call var_call_tail | ε
  expr_ptr visit_var_call_tail(derivation_node* node, expr_ptr left) {
    if (node->children[0]->symbol == epsilon) return left;

    optional<expr_ptr> object = std::move(left);
    token name = *node->children[1]->node_token;

    if (node->children[2]->children[0]->symbol == epsilon) {
      // Property access
      left = make_unique<var_expr>(std::move(object), name);
    } else {
      // Method call
      vector<expr_ptr> args = visit_finish_call(node->children[2].get());
      left = make_unique<call_expr>(std::move(object), name, std::move(args));
    }

    if (node->children.size() > 3) {
      return visit_var_call_tail(node->children[3].get(), std::move(left));
    }

    return left;
  }

  // finish_call -> LPAREN args RPAREN | ε
  vector<expr_ptr> visit_finish_call(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};
    return visit_args(node->children[1].get());
  }

  // args -> expr_list | ε
  vector<expr_ptr> visit_args(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};
    return visit_expr_list(node->children[0].get());
  }

  // expr_list -> expression expr_list_tail | ε
  vector<expr_ptr> visit_expr_list(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<expr_ptr> args;
    if (node->children[0]->children[0]->symbol != epsilon)
      args.push_back(visit_expression(node->children[0].get()));

    if (node->children.size() > 1 &&
        node->children[1]->children[0]->symbol != epsilon) {
      auto more = visit_expr_list_tail(node->children[1].get());
      args.insert(args.end(), std::make_move_iterator(more.begin()),
                  std::make_move_iterator(more.end()));
    }

    return args;
  }

  // expr_list_tail -> COMMA expression expr_list_tail | ε
  vector<expr_ptr> visit_expr_list_tail(derivation_node* node) {
    if (node->children[0]->symbol == epsilon) return {};

    vector<expr_ptr> args;
    if (node->children[1]->children[0]->symbol != epsilon)
      args.push_back(visit_expression(node->children[1].get()));

    if (node->children.size() > 2 &&
        node->children[2]->children[0]->symbol != epsilon) {
      auto more = visit_expr_list_tail(node->children[2].get());
      args.insert(args.end(), std::make_move_iterator(more.begin()),
                  std::make_move_iterator(more.end()));
    }

    return args;
  }

  // primary -> FALSE | TRUE | T_NULL | NUMBER | STRING | IDENTIFIER finish_call
  // |
  //            LPAREN expression RPAREN | KW_NEW IDENTIFIER LPAREN args RPAREN
  expr_ptr visit_primary(derivation_node* node) {
    auto child = node->children[0].get();

    if (child->symbol == "FALSE" || child->symbol == "TRUE" ||
        child->symbol == "T_NULL" || child->symbol == "NUMBER" ||
        child->symbol == "STRING") {
      return make_unique<literal_expr>(child->node_token->get_literal());
    } else if (child->symbol == "IDENTIFIER") {
      token name = *child->node_token;
      if (node->children.size() > 1 &&
          node->children[1]->children[0]->symbol != epsilon) {
        // Function call
        vector<expr_ptr> args =
            visit_args(node->children[1]->children[1].get());
        return make_unique<call_expr>(nullopt, name, std::move(args));
      } else {
        // Variable reference
        return make_unique<var_expr>(nullopt, name);
      }
    } else if (child->symbol == "LPAREN") {
      return visit_expression(node->children[1].get());
    } else if (child->symbol == "KW_NEW") {
      token type_name = *node->children[1]->node_token;
      vector<expr_ptr> args = visit_args(node->children[3].get());
      return make_unique<new_expr>(type_name, std::move(args));
    }

    error(*child->node_token, "Invalid primary expression");
    return nullptr;
  }

  static parse_error error(token x, std::string message) {
    internal::error(x, message, "parser");
    return parse_error(message);
  }
};

}  // namespace gen_parser

}  // namespace hulk

#endif  // HULK_AST_BUILDER_HPP