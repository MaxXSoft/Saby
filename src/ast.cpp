#include "ast.h"

#include <iostream>

int IdentifierAST::CodeGen() {
    std::cout << "IdentifierAST(" << id_ << ", " << type_ << ")";
    return 0;
}

int VariableAST::CodeGen() {
    std::cout << "VariableAST(";
    if (definition_) definition_->CodeGen();
    std::cout << ", ";
    if (next_def_) next_def_->CodeGen();
    std::cout << ", " << type_ << ")";
    return 0;
}

int NumberAST::CodeGen() {
    std::cout << "NumberAST(" << value_ << ")";
    return 0;
}

int DecimalAST::CodeGen() {
    std::cout << "DecimalAST(" << value_ << ")";
    return 0;
}

int StringAST::CodeGen() {
    std::cout << "StringAST(" << str_ << ")";
    return 0;
}

int BinaryExpressionAST::CodeGen() {
    std::cout << "BinaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (lhs_) lhs_->CodeGen();
    std::cout << ", ";
    if (rhs_) rhs_->CodeGen();
    std::cout << ")";
    return 0;
}

int UnaryExpressionAST::CodeGen() {
    std::cout << "UnaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (operand_) operand_->CodeGen();
    std::cout << ")";
    return 0;
}

int CallAST::CodeGen() {
	std::cout << "CallAST(";
	callee_->CodeGen();
	std::cout << ", ";
    for (const auto &i : args_) {
        if (i) i->CodeGen();
        std::cout << ". ";
    }
    std::cout << ")";
    return 0;
}

int BlockAST::CodeGen() {
    std::cout << "{";
    for (const auto &i : expr_list_) {
        if (i) i->CodeGen();
        std::cout << "; ";
    }
    std::cout << "}";
    return 0;
}

int FunctionAST::CodeGen() {
    std::cout << "FunctionAST(";
    for (const auto &i : args_) {
        if (i) i->CodeGen();
        std::cout << ". ";
    }
	std::cout << ", ";
	std::cout << return_type_ << ", ";
    if (body_) body_->CodeGen();
    std::cout << ")";
    return 0;
}

int AsmAST::CodeGen() {
    std::cout << "AsmAST(" << asm_str_ << ")";
    return 0;
}

int IfAST::CodeGen() {
    std::cout << "IfAST(";
    if (cond_) cond_->CodeGen();
    std::cout << ", ";
    if (then_) then_->CodeGen();
    std::cout << ", ";
    if (else_then_) else_then_->CodeGen();
    std::cout << ")";
    return 0;
}

int WhileAST::CodeGen() {
    std::cout << "WhileAST(";
    if (cond_) cond_->CodeGen();
    std::cout << ", ";
    if (body_) body_->CodeGen();
    std::cout << ")";
    return 0;
}

int ControlFlowAST::CodeGen() {
    std::cout << "ControlFlowAST(" << type_ << ")";
    return 0;
}

int SingleWordAST::CodeGen() {
    std::cout << "SingleWordAST(" << type_ << ", ";
    if (value_) value_->CodeGen();
    std::cout << ")";
    return 0;
}
