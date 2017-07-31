// Semantic Analysis

#include "ast.h"

namespace {

//

} // namespace

// TODO: should remove some extra 'env' assignment expression

TypeValue IdentifierAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeId(id_, type_);
    env = ana.env();
    return ret;
}

TypeValue VariableAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeVar(defs_, type_);
    env = ana.env();
    return ret;
}

TypeValue NumberAST::SemaAnalyze(Analyzer &ana) {
    return kNumber;
}

TypeValue DecimalAST::SemaAnalyze(Analyzer &ana) {
    return kFloat;
}

TypeValue StringAST::SemaAnalyze(Analyzer &ana) {
    return kString;
}

TypeValue BinaryExpressionAST::SemaAnalyze(Analyzer &ana) {
    // TODO: did not consider situation like 'a + 1 = a + 1'
    auto ret = ana.AnalyzeBinExpr(operator_id_, lhs_->SemaAnalyze(ana), rhs_->SemaAnalyze(ana));
    env = ana.env();
    return ret;
}

TypeValue UnaryExpressionAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeUnaExpr(operator_id_, operand_->SemaAnalyze(ana));
    env = ana.env();
    return ret;
}

TypeValue CallAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeCall(callee_, args_);
    env = ana.env();
    return ret;
}

TypeValue BlockAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeBlock(expr_list_);
    env = ana.nested_env();
    return ret;
}

TypeValue FunctionAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeFunc(args_, return_type_, body_);
    env = ana.nested_env();
    return ret;
}

TypeValue AsmAST::SemaAnalyze(Analyzer &ana) {
    env = ana.env();   // it's necessary
    return kVoid;
}

TypeValue IfAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeIf(cond_, then_, else_then_);
    env = ana.env();
    return ret;
}

TypeValue WhileAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeWhile(cond_, body_);
    env = ana.env();
    return ret;
}

TypeValue ControlFlowAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeCtrlFlow(type_, value_);
    env = ana.env();
    return ret;
}

TypeValue ExternalAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeExtern(type_, libs_);
    env = ana.env();
    return ret;
}

