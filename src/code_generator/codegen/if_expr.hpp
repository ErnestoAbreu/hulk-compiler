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
            if (!condition_value) {
                return nullptr;
            }

            // Convert condition to boolean
            llvm::Value* condition_bool = Builder->CreateICmpNE(
                condition_value,
                llvm::ConstantInt::get(condition_value->getType(), 0),
                "ifcond"
            );

            llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

            // Create basic blocks for the then, elif, else, and merge cases
            llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*TheContext, "if.then", TheFunction);
            llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*TheContext, "if.end", TheFunction);

            // Create vectors for elif conditions and bodies
            std::vector<llvm::BasicBlock*> elif_condBBs;
            std::vector<llvm::BasicBlock*> elif_bodyBBs;
            for (size_t i = 0; i < elif_branchs.size(); ++i) {
                elif_condBBs.push_back(llvm::BasicBlock::Create(*TheContext, "if.elifcond" + std::to_string(i), TheFunction));
                elif_bodyBBs.push_back(llvm::BasicBlock::Create(*TheContext, "if.elifbody" + std::to_string(i), TheFunction));
            }

            llvm::BasicBlock* elseBB = else_branch ? 
                llvm::BasicBlock::Create(*TheContext, "if.else", TheFunction) : 
                mergeBB;

            // Emit the initial conditional branch
            llvm::BasicBlock* firstElseBB = elif_branchs.empty() ? elseBB : elif_condBBs[0];
            Builder->CreateCondBr(condition_bool, thenBB, firstElseBB);

            // Emit then block
            Builder->SetInsertPoint(thenBB);
            llvm::Value* then_value = then_branch->codegen();
            if (!then_value) {
                return nullptr;
            }
            Builder->CreateBr(mergeBB);
            thenBB = Builder->GetInsertBlock();  // Update thenBB in case it was modified

            // Emit elif blocks
            std::vector<llvm::Value*> elif_values;
            for (size_t i = 0; i < elif_branchs.size(); ++i) {
                // Emit elif condition block
                Builder->SetInsertPoint(elif_condBBs[i]);
                llvm::Value* elif_condValue = elif_branchs[i].first->codegen();
                if (!elif_condValue) {
                    return nullptr;
                }
                llvm::Value* elif_condBool = Builder->CreateICmpNE(
                    elif_condValue,
                    llvm::ConstantInt::get(elif_condValue->getType(), 0),
                    "elifcond"
                );

                llvm::BasicBlock* nextCondBB = (i + 1 < elif_branchs.size()) ? 
                    elif_condBBs[i + 1] : 
                    elseBB;
                Builder->CreateCondBr(elif_condBool, elif_bodyBBs[i], nextCondBB);

                // Emit elif body block
                Builder->SetInsertPoint(elif_bodyBBs[i]);
                llvm::Value* elif_value = elif_branchs[i].second->codegen();
                if (!elif_value) {
                    return nullptr;
                }
                elif_values.push_back(elif_value);
                Builder->CreateBr(mergeBB);
                elif_bodyBBs[i] = Builder->GetInsertBlock();  // Update in case it was modified
            }

            // Emit else block if it exists
            llvm::Value* else_value = nullptr;
            if (elseBB && elseBB != mergeBB) {
                Builder->SetInsertPoint(elseBB);
                else_value = (*else_branch)->codegen();
                if (!else_value) {
                    return nullptr;
                }
                Builder->CreateBr(mergeBB);
                elseBB = Builder->GetInsertBlock();  // Update elseBB in case it was modified
            }

            // Emit merge block
            Builder->SetInsertPoint(mergeBB);

            // Only create PHI node if there are multiple possible incoming values
            if (then_value && (elif_values.size() > 0 || else_value)) {
                llvm::PHINode* phi = Builder->CreatePHI(then_value->getType(), 
                    1 + elif_values.size() + (elseBB && elseBB != mergeBB ? 1 : 0), 
                    "iftmp");

                phi->addIncoming(then_value, thenBB);

                for (size_t i = 0; i < elif_values.size(); ++i) {
                    phi->addIncoming(elif_values[i], elif_bodyBBs[i]);
                }

                if (elseBB && elseBB != mergeBB && else_value) {
                    phi->addIncoming(else_value, elseBB);
                }

                return phi;
            }

            // If no PHI node was needed, return the then value (or null if no then block executed)
            return then_value;
        }

    } // namespace ast
} // namespace hulk

#endif // HULK_CODEGEN_IF_EXPR_HPP