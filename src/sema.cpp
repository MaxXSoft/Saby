// Semantic Analysis

#include "ast.h"

// TODO: should remove some extra 'env' assignment expression

TypeValue IdentifierAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeId(id_, type_);
    env = ana.env();
    return ret;
}

TypeValue VariableAST::SemaAnalyze(Analyzer &ana) {
    VarTypeList var_type;
    for (const auto &i : defs_) {
        if (!i.second) return kTypeError;   // initialization list is empty
        var_type.push_back({i.first, i.second->SemaAnalyze(ana)});
    }
    auto ret = ana.AnalyzeVar(var_type, type_);
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
    TypeList args_type;
    for (const auto &i : args_) {
        args_type.push_back(i->SemaAnalyze(ana));
    }
    auto ret = ana.AnalyzeCall(callee_->SemaAnalyze(ana), args_type);
    env = ana.env();
    return ret;
}

TypeValue BlockAST::SemaAnalyze(Analyzer &ana) {
    ana.NewEnvironment();

    for (const auto &i : expr_list_) {
        if (i->SemaAnalyze(ana) == kTypeError) return kTypeError;
    }

    ana.RestoreEnvironment();
    env = ana.nested_env();
    return kVoid;
}

TypeValue FunctionAST::SemaAnalyze(Analyzer &ana) {
    ana.NewEnvironment();

    TypeList args_type;
    for (const auto &i : args_) {
        args_type.push_back(i->SemaAnalyze(ana));
    }
    auto ret = ana.AnalyzeFunc(args_type, return_type_);

    if (body_->SemaAnalyze(ana) == kTypeError) return kTypeError;

    ana.RestoreEnvironment();
    env = ana.nested_env();
    return ret;
}

TypeValue AsmAST::SemaAnalyze(Analyzer &ana) {
    env = ana.env();   // it's necessary
    return kVoid;
}

TypeValue IfAST::SemaAnalyze(Analyzer &ana) {
    if (cond_->SemaAnalyze(ana) == kTypeError) return kTypeError;
    if (then_->SemaAnalyze(ana) == kTypeError) return kTypeError;
    if (else_then_->SemaAnalyze(ana) == kTypeError) return kTypeError;
    env = ana.env();
    return kVoid;
}

TypeValue WhileAST::SemaAnalyze(Analyzer &ana) {
    if (cond_->SemaAnalyze(ana) == kTypeError) return kTypeError;
    if (body_->SemaAnalyze(ana) == kTypeError) return kTypeError;
    env = ana.env();
    return kVoid;
}

TypeValue ControlFlowAST::SemaAnalyze(Analyzer &ana) {
    auto value = value_ ? value_->SemaAnalyze(ana) : kTypeError;
    auto ret = ana.AnalyzeCtrlFlow(type_, value);
    env = ana.env();
    return ret;
}

TypeValue ExternalAST::SemaAnalyze(Analyzer &ana) {
    auto ret = ana.AnalyzeExtern(type_, libs_);
    env = ana.env();
    return ret;
}

