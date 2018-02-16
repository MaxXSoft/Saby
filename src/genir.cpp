#include "ast.h"

#include <set>
#include <cassert>

#include "lexer.h"
#include "type.h"

namespace {

using Operator = QuadSSA::Operator;

inline SSAPtr GetValueByType(int type, int number) {
    assert(type == kNumber || type == kFloat);
    if (type == kNumber) {
        return std::make_shared<ValueSSA>(static_cast<long long>(number));
    }
    else {
        return std::make_shared<ValueSSA>(static_cast<double>(number));
    }
}

Operator GetOperator(int operator_id) {
    assert(operator_id < kAssign);
    if (operator_id <= kPow) {
        return static_cast<Operator>(operator_id <= kConvStr ?
                                     operator_id + 3 :
                                     operator_id + 2);
    }
    else if (operator_id >= kLess && operator_id <= kNeq) {
        return static_cast<Operator>(operator_id);
    }
    else {   // kInc or kDec
        return kInc ? Operator::Add : Operator::Sub;
    }
}

} // namespace

// TODO: check for unused value

SSAPtr IdentifierAST::GenIR(IRBuilder &irb) {
    if (type_ == -1) {   // variable use
        auto block_id = irb.GetCurrentBlock()->id();
        // get id recursively
        auto var_ssa = irb.ReadVariable(id_, block_id);
        return var_ssa;
    }
    else {   // function argument list
        // do nothing, FunctionAST will generate correct SSA IR
        return nullptr;
    }
}

SSAPtr VariableAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    for (const auto &i : defs_) {
        auto value = i.second->GenIR(irb);
        auto var_ssa = irb.NewVariable(i.first, value);
        cur_block->AddValue(var_ssa);
    }
    return nullptr;   // return nothing
}

SSAPtr NumberAST::GenIR(IRBuilder &irb) {
    return std::make_shared<ValueSSA>(value_);
}

SSAPtr DecimalAST::GenIR(IRBuilder &irb) {
    return std::make_shared<ValueSSA>(value_);
}

SSAPtr StringAST::GenIR(IRBuilder &irb) {
    return std::make_shared<ValueSSA>(str_);
}

SSAPtr BinaryExpressionAST::GenIR(IRBuilder &irb) {
    SSAPtr value;
    auto cur_block = irb.GetCurrentBlock();
    if (operator_id_ == kAssign) {
        // like 'a = b + 2'
        const auto &lhs_id = static_cast<IdentifierAST *>(lhs_.get())->id();
        value = irb.NewVariable(lhs_id, rhs_->GenIR(irb));
    }
    else if (operator_id_ > kAssign) {
        // like 'a += 1'
        auto op = GetOperator(operator_id_ - kAssign);
        const auto &lhs_id = static_cast<IdentifierAST *>(lhs_.get())->id();
        // get old value
        auto old_var = irb.ReadVariable(lhs_id, cur_block->id());
        // generate quad_ssa & new value
        auto quad = std::make_shared<QuadSSA>(op, old_var, rhs_->GenIR(irb));
        value = irb.NewVariable(lhs_id, quad);
    }
    else {   // operator_id (>= kAnd && <= kPow && != kNot)
        // like 'a * 3'
        auto op = GetOperator(operator_id_);
        auto lhs_ssa = lhs_->GenIR(irb);
        auto rhs_ssa = rhs_->GenIR(irb);
        auto quad = std::make_shared<QuadSSA>(op, lhs_ssa, rhs_ssa);
        value = irb.NewVariable("__tmp", quad);
    }
    // add to block
    cur_block->AddValue(value);
    return value;
}

SSAPtr UnaryExpressionAST::GenIR(IRBuilder &irb) {
    SSAPtr value;
    auto cur_block = irb.GetCurrentBlock();
    auto opr_ssa = operand_->GenIR(irb);
    switch (operator_id_) {
        case kConvNum: case kConvDec: case kConvStr: {
            // like '(string)1'
            auto op = GetOperator(operator_id_);
            auto quad = std::make_shared<QuadSSA>(op, opr_ssa, nullptr);
            value = irb.NewVariable("__tmp", quad);
            break;
        }
        case kNot: {
            // like '~a'
            auto op = QuadSSA::Operator::Not;
            auto quad = std::make_shared<QuadSSA>(op, opr_ssa, nullptr);
            value = irb.NewVariable("__tmp", quad);
            break;
        }
        case kSub: {
            // like '-a'
            auto op = QuadSSA::Operator::Sub;
            // generate '0 - a'
            auto num_value = GetValueByType(operand_type_, 0);
            auto quad = std::make_shared<QuadSSA>(op, num_value, opr_ssa);
            value = irb.NewVariable("__tmp", quad);
            break;
        }
        case kInc: case kDec: {
            // like '++a'
            using Operator = QuadSSA::Operator;
            auto op = operator_id_ == kInc ? Operator::Add : Operator::Sub;
            auto num_value = GetValueByType(operand_type_, 1);
            const auto &id = static_cast<IdentifierAST *>(operand_.get())->id();
            // get old value
            auto old_var = irb.ReadVariable(id, cur_block->id());
            // generate 'a = a + 1' or 'a = a - 1'
            auto quad = std::make_shared<QuadSSA>(op, old_var, num_value);
            value = irb.NewVariable(id, quad);
            break;
        }
    }
    // add to block
    cur_block->AddValue(value);
    return value;
}

SSAPtr CallAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    // get callee
    auto callee_ssa = callee_->GenIR(irb);
    auto call_ssa = std::make_shared<CallSSA>(callee_ssa);
    // add arguments to block and call_ssa
    for (int i = 0; i < args_.size(); ++i) {
        auto arg_ssa = args_[i]->GenIR(irb);
        auto setter = std::make_shared<ArgSetterSSA>(i, arg_ssa);
        cur_block->AddValue(setter);
        call_ssa->AddArg(setter);
    }
    cur_block->AddValue(call_ssa);
    // get return value
    SSAPtr value = nullptr;
    if (ret_type_ != kVoid) {
        auto rtn_getter = std::make_shared<RtnGetterSSA>(call_ssa);
        value = irb.NewVariable("__rtn", rtn_getter);
        cur_block->AddValue(value);
    }
    return value;
}

SSAPtr BlockAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.NewBlock();
    // handle pred
    const auto &pred = irb.pred_value();
    if (pred) {
        cur_block->AddPred(pred);
        // seal block because predecessor has been determined
        irb.SealBlock(cur_block);
    }
    // generate body
    for (const auto &i : expr_list_) {
        i->GenIR(irb);
    }
    return cur_block;
}

SSAPtr FunctionAST::GenIR(IRBuilder &irb) {
    auto old_block = irb.GetCurrentBlock();
    // generate function entry
    auto cur_block = irb.NewBlock();
    irb.SealBlock(cur_block);
    for (int i = 0; i < args_.size(); ++i) {
        const auto &id = static_cast<IdentifierAST *>(args_[i].get())->id();
        // generate argument getter
        auto getter_ssa = std::make_shared<ArgGetterSSA>(i);
        auto var_ssa = irb.NewVariable(id, getter_ssa);
        cur_block->AddValue(var_ssa);
    }
    // generate id '@'
    auto self_ssa = irb.NewVariable("@", cur_block);
    cur_block->AddValue(self_ssa);   // TODO: self-reference loop?
    // generate function body
    irb.set_pred_value(cur_block);
    auto body_ssa = body_->GenIR(irb);
    irb.set_pred_value(nullptr);
    // add 'return' in the end of function anyway
    auto body_end_block = irb.GetCurrentBlock();
    body_end_block->AddValue(std::make_shared<ReturnSSA>(nullptr));
    // generate jump statement & add to entry
    auto jump_ssa = std::make_shared<JumpSSA>(body_ssa, nullptr);
    cur_block->AddValue(jump_ssa);
    cur_block->set_is_func(true);
    // switch back to old block
    irb.SwitchCurrentBlock(old_block->id());
    return cur_block;
}

SSAPtr AsmAST::GenIR(IRBuilder &irb) {
    auto asm_ssa = std::make_shared<AsmSSA>(asm_str_);
    // NOTICE: do not remove the inline-asm during optimization
    irb.GetCurrentBlock()->AddValue(asm_ssa);
    return nullptr;
}

SSAPtr IfAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    // generate condition expression
    auto cond_ssa = cond_->GenIR(irb);
    // set pred
    irb.set_pred_value(cur_block);
    // generate if-else body
    SSAPtr if_block = then_->GenIR(irb);
    SSAPtr else_block = nullptr, else_end_block = nullptr;
    if (else_then_) {
        // handle 'else-if' structure separately
        if (else_then_->type() == ExpressionAST::ASTType::If) {
            auto new_block = irb.NewBlock();
            new_block->AddPred(cur_block);
            else_block = new_block;
            irb.SealBlock(else_block);
            else_end_block = else_then_->GenIR(irb);
        }
        else {   // else_then_->type() == ASTType::Block
            else_block = else_then_->GenIR(irb);
            else_end_block = else_block;
        }
    }
    // reset pred
    irb.set_pred_value(nullptr);
    // generate end block & add preds
    auto end_block = irb.NewBlock();
    end_block->AddPred(if_block);
    end_block->AddPred(else_end_block ? else_end_block : cur_block);
    irb.SealBlock(end_block);
    // generate jump statements
    auto jump_cond = std::make_shared<JumpSSA>(if_block, cond_ssa);
    auto jump_end = std::make_shared<JumpSSA>(end_block, nullptr);
    cur_block->AddValue(jump_cond);
    auto if_block_ptr = static_cast<BlockSSA *>(if_block.get());
    if_block_ptr->AddValue(jump_end);
    if (else_block) {
        auto jump_else = std::make_shared<JumpSSA>(else_block, nullptr);
        cur_block->AddValue(jump_else);
        auto else_end_ptr = static_cast<BlockSSA *>(else_end_block.get());
        else_end_ptr->AddValue(jump_end);
    }
    else {
        cur_block->AddValue(jump_end);
    }
    return end_block;
}

SSAPtr WhileAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    // generate entry & condition expression
    auto while_entry = irb.NewBlock();
    while_entry->AddPred(cur_block);
    auto cond_ssa = cond_->GenIR(irb);
    // generate & seal end block
    auto while_end = irb.NewBlock();
    while_end->AddPred(while_entry);
    irb.SealBlock(while_end);
    // set pred & break/continue info
    irb.set_pred_value(while_entry);
    irb.break_cont_stack().push({while_end, while_entry});
    // generate while-body and get the end of body
    SSAPtr while_body = body_->GenIR(irb);
    SSAPtr while_body_end = irb.GetCurrentBlock();
    // restore pred value & 'break_cont_stack'
    irb.break_cont_stack().pop();
    irb.set_pred_value(nullptr);
    // set the pred of entry & seal entry block
    while_entry->AddPred(while_body_end);
    irb.SealBlock(while_entry);
    // generate jump statements
    auto jump_entry = std::make_shared<JumpSSA>(while_entry, nullptr);
    auto jump_body = std::make_shared<JumpSSA>(while_body, cond_ssa);
    auto jump_end = std::make_shared<JumpSSA>(while_end, nullptr);
    // add jump statements
    cur_block->AddValue(jump_entry);
    while_entry->AddValue(jump_body);
    while_entry->AddValue(jump_end);
    auto body_end_ptr = static_cast<BlockSSA *>(while_body_end.get());
    body_end_ptr->AddValue(jump_entry);
    // switch current block to 'while_end'
    irb.SwitchCurrentBlock(while_end->id());
    return nullptr;
}

SSAPtr ControlFlowAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    SSAPtr value = nullptr;
    switch (type_) {
        case kReturn: {
            SSAPtr value_ssa = value_ ? value_->GenIR(irb) : nullptr;
            value = std::make_shared<ReturnSSA>(value_ssa);
            break;
        }
        case kBreak: {
            auto &stack = irb.break_cont_stack();
            if (!stack.empty()) {
                const auto &while_end = stack.top().first;
                auto jump_ssa = std::make_shared<JumpSSA>(while_end, nullptr);
                value = jump_ssa;
            }
            break;
        }
        case kContinue: {
            auto &stack = irb.break_cont_stack();
            if (!stack.empty()) {
                const auto &while_entry = stack.top().second;
                auto jump_ssa = std::make_shared<JumpSSA>(while_entry, nullptr);
                value = jump_ssa;
            }
            break;
        }
    }
    if (value) cur_block->AddValue(value);
    return nullptr;
}

SSAPtr ExternalAST::GenIR(IRBuilder &irb) {
    auto lib_env = env()->outermost();
    if (type_ == kImport) {
        auto cur_block = irb.GetCurrentBlock();
        const auto &loaded_libs = *lib_env->loaded_libs();
        for (const auto &i : loaded_libs) {
            // create external function ssa
            auto ext_func = std::make_shared<ExternFuncSSA>(i);
            // get func name & generate var
            // TODO: consider the efficiency of 'substr'
            auto &&func_name = i.substr(i.find('.') + 1);
            auto var_ssa = irb.NewVariable(func_name, ext_func);
            cur_block->AddValue(var_ssa);
        }
        // save library info
        for (const auto &i : libs_) {
            irb.imported_libs().push_back(i);
        }
    }
    else {   // type_ == kExport
        const auto &exported_funcs = *lib_env->exported_funcs();
        irb.set_exported_funcs(exported_funcs);
    }
    return nullptr;
}
