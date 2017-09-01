#include "ast.h"

#include <iostream>

#include "irbuilder.h"

namespace {

IRBuilder irb;

} // namespace

SSAPtr IdentifierAST::GenIR() {
    std::cout << "IdentifierAST(" << id_ << ", " << type_ << ")";
    return nullptr;
}

SSAPtr VariableAST::GenIR() {
    std::cout << "VariableAST(";
    for (const auto &i : defs_) {
        std::cout << "(" << i.first << ", ";
        i.second->GenIR();
        std::cout << "). ";
    }
    std::cout << ", " << type_ << ")";
    return nullptr;
}

SSAPtr NumberAST::GenIR() {
    std::cout << "NumberAST(" << value_ << ")";
    return nullptr;
}

SSAPtr DecimalAST::GenIR() {
    std::cout << "DecimalAST(" << value_ << ")";
    return nullptr;
}

SSAPtr StringAST::GenIR() {
    std::cout << "StringAST(" << str_ << ")";
    return nullptr;
}

SSAPtr BinaryExpressionAST::GenIR() {
    std::cout << "BinaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (lhs_) lhs_->GenIR();
    std::cout << ", ";
    if (rhs_) rhs_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr UnaryExpressionAST::GenIR() {
    std::cout << "UnaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (operand_) operand_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr CallAST::GenIR() {
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

SSAPtr BlockAST::GenIR() {
    std::cout << "{";
    for (const auto &i : expr_list_) {
        if (i) i->GenIR();
        std::cout << "; ";
    }
    std::cout << "}";
    return nullptr;
}

SSAPtr FunctionAST::GenIR() {
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

SSAPtr AsmAST::GenIR() {
    std::cout << "AsmAST(" << asm_str_ << ")";
    return nullptr;
}

SSAPtr IfAST::GenIR() {
    std::cout << "IfAST(";
    if (cond_) cond_->GenIR();
    std::cout << ", ";
    if (then_) then_->GenIR();
    std::cout << ", ";
    if (else_then_) else_then_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr WhileAST::GenIR() {
    std::cout << "WhileAST(";
    if (cond_) cond_->GenIR();
    std::cout << ", ";
    if (body_) body_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr ControlFlowAST::GenIR() {
    std::cout << "ControlFlowAST(" << type_ << ", ";
    if (value_) value_->GenIR();
    std::cout << ")";
    return nullptr;
}

SSAPtr ExternalAST::GenIR() {
    std::cout << "ExternalAST(" << type_ << ", ";
    for (const auto &i : libs_) {
        std::cout << i << ". ";
    }
    std::cout << ")";
    return nullptr;
}
