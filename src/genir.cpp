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
        // TODO: how to handle external call?
        auto block_id = irb.GetCurrentBlock()->id();
        // get id recursively
        auto var_id = env()->GetIDRef(id_);
        auto var_ssa = irb.ReadVariable(var_id, block_id);
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
        auto var_ssa = irb.NewVariable(value);
        // save var_id
        env()->id_table()[i.first] = var_ssa->id();
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
        auto var_ssa = irb.NewVariable(rhs_->GenIR(irb));
        const auto &lhs_id = static_cast<IdentifierAST *>(lhs_.get())->id();
        // update variable id
        env()->GetIDRef(lhs_id) = var_ssa->id();
        value = var_ssa;
    }
    else if (operator_id_ > kAssign) {
        // like 'a += 1'
        auto op = GetOperator(operator_id_ - kAssign);
        const auto &lhs_id = static_cast<IdentifierAST *>(lhs_.get())->id();
        // get variable id & old value
        auto &var_id = env()->GetIDRef(lhs_id);
        auto old_var = irb.ReadVariable(var_id, cur_block->id());
        // generate quad_ssa & new value
        auto quad = std::make_shared<QuadSSA>(op, old_var, rhs_->GenIR(irb));
        auto new_var = irb.NewVariable(quad);
        // update variable id
        var_id = new_var->id();
        value = new_var;
    }
    else {   // operator_id (>= kAnd && <= kPow && != kNot)
        // like 'a * 3'
        auto op = GetOperator(operator_id_);
        auto lhs_ssa = lhs_->GenIR(irb);
        auto rhs_ssa = rhs_->GenIR(irb);
        auto quad = std::make_shared<QuadSSA>(op, lhs_ssa, rhs_ssa);
        value = irb.NewVariable(quad);
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
            value = irb.NewVariable(quad);
            break;
        }
        case kNot: {
            // like '~a'
            auto op = QuadSSA::Operator::Not;
            auto quad = std::make_shared<QuadSSA>(op, opr_ssa, nullptr);
            value = irb.NewVariable(quad);
            break;
        }
        case kSub: {
            // like '-a'
            auto op = QuadSSA::Operator::Sub;
            // generate '0 - a'
            auto num_value = GetValueByType(operand_type_, 0);
            auto quad = std::make_shared<QuadSSA>(op, num_value, opr_ssa);
            value = irb.NewVariable(quad);
            break;
        }
        case kInc: case kDec: {
            // like '++a'
            using Operator = QuadSSA::Operator;
            auto op = operator_id_ == kInc ? Operator::Add : Operator::Sub;
            auto num_value = GetValueByType(operand_type_, 1);
            const auto &id = static_cast<IdentifierAST *>(operand_.get())->id();
            // get variable id
            auto &var_id = env()->GetIDRef(id);
            auto old_var = irb.ReadVariable(var_id, cur_block->id());
            // generate 'a = a + 1' or 'a = a - 1'
            auto quad = std::make_shared<QuadSSA>(op, old_var, num_value);
            auto new_var = irb.NewVariable(quad);
            // update variable id
            var_id = new_var->id();
            value = new_var;
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
    auto rtn_getter = std::make_shared<RtnGetterSSA>(call_ssa);
    auto value = irb.NewVariable(rtn_getter);
    cur_block->AddValue(value);   // TODO: how to handle () => void?
    return value;
}

SSAPtr BlockAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.NewBlock();
    // handle pred
    const auto &pred = irb.pred_value();
    if (pred) cur_block->AddPred(pred);
    // generate body
    for (const auto &i : expr_list_) {
        i->GenIR(irb);
    }
    // TODO: make sure you should do this during this process
    // seal block
    irb.SealBlock(cur_block);
    return cur_block;
}

SSAPtr FunctionAST::GenIR(IRBuilder &irb) {
    auto old_block = irb.GetCurrentBlock();
    // generate function entry
    auto cur_block = irb.NewBlock();
    // TODO: implement GetBlockBySSA (NOTE: do this in code gen proc)
    for (int i = 0; i < args_.size(); ++i) {
        auto id_ast = static_cast<IdentifierAST *>(args_[i].get());
        auto getter_ssa = std::make_shared<ArgGetterSSA>(i);
        auto var_ssa = irb.NewVariable(getter_ssa);
        // save argument var def
        env()->id_table()[id_ast->id()] = var_ssa->id();
        cur_block->AddValue(var_ssa);
    }
    // generate id '@'
    auto self_ssa = irb.NewVariable(cur_block);
    env()->id_table()["@"] = self_ssa->id();
    cur_block->AddValue(self_ssa);   // TODO: self-reference loop?
    // generate function body
    irb.set_pred_value(cur_block);
    auto body_ssa = body_->GenIR(irb);
    irb.set_pred_value(nullptr);
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
    SSAPtr else_block = nullptr;
    if (else_then_) else_block = else_then_->GenIR(irb);
    // reset pred
    irb.set_pred_value(nullptr);
    // generate end block & add preds
    auto end_block = irb.NewBlock();
    end_block->AddPred(if_block);
    end_block->AddPred(else_block ? else_block : cur_block);
    // generate jump statements
    auto jump_cond = std::make_shared<JumpSSA>(if_block, cond_ssa);
    auto jump_end = std::make_shared<JumpSSA>(end_block, nullptr);
    cur_block->AddValue(jump_cond);
    if (else_block) {
        auto jump_else = std::make_shared<JumpSSA>(else_block, nullptr);
        cur_block->AddValue(jump_else);
        auto else_block_ptr = static_cast<BlockSSA *>(else_block.get());
        else_block_ptr->AddValue(jump_end);
    }
    else {
        cur_block->AddValue(jump_end);
    }
    // seal block
    irb.SealBlock(cur_block);
    return nullptr;
}

SSAPtr WhileAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    // generate entry & condition expression
    auto while_entry = irb.NewBlock();
    while_entry->AddPred(cur_block);
    auto cond_ssa = cond_->GenIR(irb);
    // generate end block
    auto while_end = irb.NewBlock();
    // set pred & break/continue info
    irb.set_pred_value(while_entry);
    irb.break_cont_stack().push({while_end, while_entry});
    // generate while-body
    auto while_body = body_->GenIR(irb);
    // restore pred value & 'break_cont_stack'
    irb.break_cont_stack().pop();
    irb.set_pred_value(nullptr);
    // set the pred of entry & end
    while_entry->AddPred(while_body);
    while_end->AddPred(while_entry);
    // generate jump statements
    auto jump_entry = std::make_shared<JumpSSA>(while_entry, nullptr);
    auto jump_body = std::make_shared<JumpSSA>(while_body, cond_ssa);
    auto jump_end = std::make_shared<JumpSSA>(while_end, nullptr);
    // add jump statements
    cur_block->AddValue(jump_entry);
    while_entry->AddValue(jump_body);
    while_entry->AddValue(jump_end);
    auto body_block_ptr = static_cast<BlockSSA *>(while_body.get());
    body_block_ptr->AddValue(jump_body);
    // seal blocks
    irb.SealBlock(cur_block);
    irb.SealBlock(while_entry);
    // switch current block to 'while_end'
    irb.SwitchCurrentBlock(while_end->id());
    return nullptr;
}

SSAPtr ControlFlowAST::GenIR(IRBuilder &irb) {
    auto cur_block = irb.GetCurrentBlock();
    SSAPtr value = nullptr;
    switch (type_) {
        case kReturn: {
            auto op = QuadSSA::Operator::Return;
            auto value_ssa = value_->GenIR(irb);
            value = std::make_shared<QuadSSA>(op, value_ssa, nullptr);
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
            auto var_ssa = irb.NewVariable(ext_func);
            // get func name & save var id
            // TODO: consider the efficiency of 'substr'
            auto &&func_name = i.substr(i.find('.') + 1);
            lib_env->id_table()[func_name] = var_ssa->id();
            cur_block->AddValue(var_ssa);
        }
        // save library info
        for (const auto &i : libs_) {
            irb.imported_libs().push_back(i);
        }
    }
    else {   // type_ == kExport
        const auto &exported_funcs = *lib_env->exported_funcs();
        for (const auto &i : exported_funcs) {
            // save function name & id
            const auto &func_id = env()->GetIDRef(i);
            irb.exported_funcs().push_back({i, func_id});
        }
    }
    return nullptr;
}
