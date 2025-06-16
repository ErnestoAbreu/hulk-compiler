#ifndef HULK_CODEGEN_WHILE_EXPR_HPP
#define HULK_CODEGEN_WHILE_EXPR_HPP 1

#include "../../ast/ast"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"

namespace hulk {

namespace ast {

llvm::Value* while_expr::codegen() {
    llvm::BasicBlock* curBB = Builder->GetInsertBlock();
    llvm::Function* TheFunction = curBB->getParent();

    llvm::BasicBlock* conditionBB =
        llvm::BasicBlock::Create(*TheContext, "while.cond", TheFunction);
    llvm::BasicBlock* bodyBB =
        llvm::BasicBlock::Create(*TheContext, "while.body", TheFunction);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*TheContext, "while.end", TheFunction);

    Builder->CreateBr(conditionBB);
    Builder->SetInsertPoint(conditionBB);

    llvm::Value* condition_value = condition->codegen();

    llvm::Value* condition_bool = Builder->CreateICmpNE(
        condition_value, llvm::ConstantInt::get(condition_value->getType(), 0),
        "while.test");
    
    Builder->CreateCondBr(condition_bool, bodyBB, endBB);

    Builder->SetInsertPoint(bodyBB);
    llvm::Value* body_value = body->codegen();

    Builder->CreateBr(conditionBB);

    Builder->SetInsertPoint(endBB);
    return body_value;
}

}  // namespace ast

}  // namespace hulk

#endif  // HULK_CODEGEN_WHILE_EXPR_HPP
