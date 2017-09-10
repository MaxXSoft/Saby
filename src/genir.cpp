#include "ast.h"

#include <iostream>

namespace {

// IRBuilder irb;

} // namespace

SSAPtr IdentifierAST::GenIR(IRBuilder &irb) {
    std::cout << "IdentifierAST(" << id_ << ", " << type_ << ")";
    return nullptr;
}

SSAPtr VariableAST::GenIR(IRBuilder &irb) {
    std::cout << "VariableAST(";
    for (const auto &i : defs_) {
        std::cout << "(" << i.first << ", ";
        i.second->GenIR();
        std::cout << "). ";
    }
    std::cout << ", " << type_ << ")";
    return nullptr;
}

SSAPtr NumberAST::GenIR(IRBuilder &irb) {
    // std::cout << "NumberAST(" << value_ << ")";
    return std::make_unique<ValueSSA>(value_);
}

SSAPtr DecimalAST::GenIR(IRBuilder &irb) {
    // std::cout << "DecimalAST(" << value_ << ")";
    return std::make_unique<ValueSSA>(value_);
}

SSAPtr StringAST::GenIR(IRBuilder &irb) {
    // std::cout << "StringAST(" << str_ << ")";
    return std::make_unique<ValueSSA>(str_);
}

SSAPtr BinaryExpressionAST::GenIR(IRBuilder &irb) {
    std::cout << "BinaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (lhs_) lhs_->GenIR();
    std::cout << ", ";
    if (rhs_) rhs_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr UnaryExpressionAST::GenIR(IRBuilder &irb) {
    std::cout << "UnaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (operand_) operand_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr CallAST::GenIR(IRBuilder &irb) {
    std::cout << "CallAST(";
    callee_->GenIR();
    std::cout << ", ";
    for (const auto &i : args_) {
        if (i) i->GenIR();
        std::cout << ". ";
    }
    std::cout << ")";
    return nullptr;
}

SSAPtr BlockAST::GenIR(IRBuilder &irb) {
    std::cout << "{";
    for (const auto &i : expr_list_) {
        if (i) i->GenIR();
        std::cout << "; ";
    }
    std::cout << "}";
    return nullptr;
}

SSAPtr FunctionAST::GenIR(IRBuilder &irb) {
    std::cout << "FunctionAST(";
    for (const auto &i : args_) {
        if (i) i->GenIR();
        std::cout << ". ";
    }
    std::cout << ", ";
    std::cout << return_type_ << ", ";
    if (body_) body_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr AsmAST::GenIR(IRBuilder &irb) {
    std::cout << "AsmAST(" << asm_str_ << ")";
    return nullptr;
}

SSAPtr IfAST::GenIR(IRBuilder &irb) {
    std::cout << "IfAST(";
    if (cond_) cond_->GenIR();
    std::cout << ", ";
    if (then_) then_->GenIR();
    std::cout << ", ";
    if (else_then_) else_then_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr WhileAST::GenIR(IRBuilder &irb) {
    std::cout << "WhileAST(";
    if (cond_) cond_->GenIR();
    std::cout << ", ";
    if (body_) body_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr ControlFlowAST::GenIR(IRBuilder &irb) {
    std::cout << "ControlFlowAST(" << type_ << ", ";
    if (value_) value_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr ExternalAST::GenIR(IRBuilder &irb) {
    std::cout << "ExternalAST(" << type_ << ", ";
    for (const auto &i : libs_) {
        std::cout << i << ". ";
    }
    std::cout << ")";
    return nullptr;
}
