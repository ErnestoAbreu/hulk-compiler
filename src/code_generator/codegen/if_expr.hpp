#ifndef HULK_CODEGEN_IF_EXPR_HPP
#define HULK_CODEGEN_IF_EXPR_HPP 1

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"  
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

#include "../../ast/ast"

namespace hulk {
    namespace ast {

        llvm::Value* if_expr::codegen() {
            llvm::Value* condition_value = condition->codegen();
            llvm::Value* condition_bool = Builder->CreateICmpNE(
                condition_value,
                llvm::ConstantInt::get(condition_value->getType(), 0),
                "ifcond"
            );

            llvm::BasicBlock* curBB = Builder->GetInsertBlock();
            llvm::Function* TheFunction = curBB->getParent();

            llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*TheContext, "if.then", TheFunction);

            std::vector<llvm::BasicBlock*> elif_condBB, elif_bodyBB;
            for (size_t i = 0; i < elif_branchs.size(); ++i) {
                elif_condBB.push_back(
                    llvm::BasicBlock::Create(*TheContext, "if.elifcond" + std::to_string(i), TheFunction)
                );
                elif_bodyBB.push_back(
                    llvm::BasicBlock::Create(*TheContext, "if.elifbody" + std::to_string(i), TheFunction)
                );
            }

            llvm::BasicBlock* elseBB = else_branch
                ? llvm::BasicBlock::Create(*TheContext, "if.else", TheFunction)
                : nullptr;
            llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*TheContext, "if.end", TheFunction);

            llvm::BasicBlock* firstElseBB = 
                !elif_branchs.empty() ? elif_condBB[0]
            : (elseBB ? elseBB : mergeBB);
            Builder->CreateCondBr(condition_bool, thenBB, firstElseBB);

            Builder->SetInsertPoint(thenBB);
            llvm::Value* then_value = then_branch->codegen();
            Builder->CreateBr(mergeBB);

            std::vector<llvm::Value*> elif_values;
            for (size_t i = 0; i < elif_branchs.size(); ++i) {
                Builder->SetInsertPoint(elif_condBB[i]);
                llvm::Value* elif_condValue = elif_branchs[i].first->codegen();
                llvm::Value* elif_condBool = Builder->CreateICmpNE(
                    elif_condValue,
                    llvm::ConstantInt::get(elif_condValue->getType(), 0),
                    "elifcond"
                );

                llvm::BasicBlock* nextCondBB = 
                    (i + 1 < elif_branchs.size())
                    ? elif_condBB[i + 1]
                    : (elseBB ? elseBB : mergeBB);

                Builder->CreateCondBr(elif_condBool,   
                                    elif_bodyBB[i],
                                    nextCondBB);

                Builder->SetInsertPoint(elif_bodyBB[i]);
                llvm::Value* elif_value = elif_branchs[i].second->codegen();
                elif_values.push_back(elif_value);
                Builder->CreateBr(mergeBB);
            }

            llvm::Value* else_value = nullptr;
            if (elseBB) {
                Builder->SetInsertPoint(elseBB);
                else_value = (*else_branch)->codegen();
                Builder->CreateBr(mergeBB);
            }

            Builder->SetInsertPoint(mergeBB);
            unsigned numIncoming = 1 + elif_values.size() + (elseBB ? 1u : 0u);
            llvm::PHINode* phi = Builder->CreatePHI(then_value->getType(), numIncoming, "iftmp");

            phi->addIncoming(then_value, thenBB);

            for (size_t i = 0; i < elif_values.size(); ++i) {
                phi->addIncoming(elif_values[i], elif_bodyBB[i]);
            }

            if (elseBB) {
                phi->addIncoming(else_value, elseBB);
            }

            return phi;
        }


    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_IF_EXPR_HPP
