#include "ast.h"

#include <iostream>

int IdentifierAST::CodeGen(EnvPtr &env) {
    std::cout << "IdentifierAST(" << id_ << ", " << type_ << ")";
    
    return 0;
}

int VariableAST::CodeGen(EnvPtr &env) {
    std::cout << "VariableAST(";
    if (definition_) definition_->CodeGen(env);
    std::cout << ", ";
    if (next_def_) next_def_->CodeGen(env);
    std::cout << ", " << type_ << ")";
    return 0;
}

int NumberAST::CodeGen(EnvPtr &env) {
    std::cout << "NumberAST(" << value_ << ")";
    return 0;
}

int DecimalAST::CodeGen(EnvPtr &env) {
    std::cout << "DecimalAST(" << value_ << ")";
    return 0;
}

int StringAST::CodeGen(EnvPtr &env) {
    std::cout << "StringAST(" << str_ << ")";
    return 0;
}

int BinaryExpressionAST::CodeGen(EnvPtr &env) {
    std::cout << "BinaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (lhs_) lhs_->CodeGen(env);
    std::cout << ", ";
    if (rhs_) rhs_->CodeGen(env);
    std::cout << ")";
    return 0;
}

int UnaryExpressionAST::CodeGen(EnvPtr &env) {
    std::cout << "UnaryExpressionAST(" << operator_id_;
    std::cout << ", ";
    if (operand_) operand_->CodeGen(env);
    std::cout << ")";
    return 0;
}

int CallAST::CodeGen(EnvPtr &env) {
	std::cout << "CallAST(";
	callee_->CodeGen(env);
	std::cout << ", ";
    for (const auto &i : args_) {
        if (i) i->CodeGen(env);
        std::cout << ". ";
    }
    std::cout << ")";
    return 0;
}

int BlockAST::CodeGen(EnvPtr &env) {
    std::cout << "{";
    for (const auto &i : expr_list_) {
        if (i) i->CodeGen(env);
        std::cout << "; ";
    }
    std::cout << "}";
    return 0;
}

int FunctionAST::CodeGen(EnvPtr &env) {
    std::cout << "FunctionAST(";
    for (const auto &i : args_) {
        if (i) i->CodeGen(env);
        std::cout << ". ";
    }
	std::cout << ", ";
	std::cout << return_type_ << ", ";
    if (body_) body_->CodeGen(env);
    std::cout << ")";
    return 0;
}

int AsmAST::CodeGen(EnvPtr &env) {
    std::cout << "AsmAST(" << asm_str_ << ")";
    return 0;
}

int IfAST::CodeGen(EnvPtr &env) {
    std::cout << "IfAST(";
    if (cond_) cond_->CodeGen(env);
    std::cout << ", ";
    if (then_) then_->CodeGen(env);
    std::cout << ", ";
    if (else_then_) else_then_->CodeGen(env);
    std::cout << ")";
    return 0;
}

int WhileAST::CodeGen(EnvPtr &env) {
    std::cout << "WhileAST(";
    if (cond_) cond_->CodeGen(env);
    std::cout << ", ";
    if (body_) body_->CodeGen(env);
    std::cout << ")";
    return 0;
}

int ControlFlowAST::CodeGen(EnvPtr &env) {
    std::cout << "ControlFlowAST(" << type_ << ")";
    return 0;
}

int SingleWordAST::CodeGen(EnvPtr &env) {
    std::cout << "SingleWordAST(" << type_ << ", ";
    if (value_) value_->CodeGen(env);
    std::cout << ")";
    return 0;
}
