// Semantic Analyzer

#include "analyzer.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

// just a limit which can simplify code generating
constexpr int kFuncMaxArgNum = 6;
constexpr TypeValue kFuncTypeBase = 131;

TypeValue GetFunctionType(const std::vector<TypeValue> &args_type, TypeValue ret_type) {
    if (ret_type == kTypeError) return kTypeError;
    TypeValue func_type = 0;
    for (const auto &i : args_type) {
        if (i == kTypeError) return kTypeError;
        func_type = func_type * kFuncTypeBase + i + 1;   // BKDR hash
    }
    func_type = func_type * kFuncTypeBase + ret_type + kFuncTypeBase;
    return func_type;
}

inline TypeValue GetFuncRetType(TypeValue func_type) {
    return (func_type - kFuncTypeBase) % kFuncTypeBase;
}

bool IsBinaryOperator(int operator_id) {
    switch (operator_id) {
        case kConvNum: case kConvDec: case kConvStr:
        case kInc: case kDec: {
            return false;
        }
        default: {
            return true;   // operator '(-)' will be regard as binary
        }
    }
}

bool CheckType(int operator_id, TypeValue type) {
    switch (operator_id) {
        case kConvNum: {
            return type == kFloat || type == kString || type == kVar;
        }
        case kConvDec: {
            return type == kNumber || type == kString || type == kVar;
        }
        case kConvStr: {
            return type == kNumber || type == kFloat || type == kVar;
        }
        case kAnd: case kXor: case kOr: case kNot:
        case kShl: case kShr: case kMod: {
            return type == kNumber;
        }
        case kAdd: case kEql: case kNeq: {
            return type == kNumber || type == kFloat ||
                   type == kString || type == kList;
        }
        case kSub: case kMul: case kDiv: case kInc: case kDec:
        case kLess: case kLesEql: case kGreat: case kGreEql: {
            return type == kNumber || type == kFloat;
        }
        case kPow: {
            return type == kFloat;
        }
        case kAssign: {
            return true;
        }
        default: {
            auto sub_as = operator_id - kAssign;
            if (sub_as < kAssign && sub_as >= kAnd) {
                return CheckType(sub_as, type);
            }
            else {
                return false;   // operator '=>' will return false
            }
        }
    }
}

} // namespace

TypeValue Analyzer::PrintError(const char *description, const char *id) {
    if (id) {
        fprintf(stderr, "\033[1manalyzer\033[0m(before line %u): "
                        "\033[31m\033[1merror:\033[0m id '%s' %s\n", 
                parser_.line_pos(), id, description);
    }
    else {
        fprintf(stderr, "\033[1manalyzer\033[0m(before line %u): "
                        "\033[31m\033[1merror:\033[0m %s\n", 
                parser_.line_pos(), description);
    }
    ++error_num_;
    return kTypeError;
}

TypeValue Analyzer::AnalyzeId(const std::string &id, TypeValue type) {
    if (type == -1) {   // identifier reference
        auto ret = env_->GetType(id);
        if (ret != kTypeError) {
            return ret;
        }
        else {
            return PrintError("has not been defined", id.c_str());
        }
    }
    else {   // function argument list
        // add id to current env
        env_->Insert(id, type);
        return type;
    }
}

TypeValue Analyzer::AnalyzeVar(const VarDefList &defs, TypeValue type) {
    auto deduced = false;   // whether type has been deduced
    for (const auto &i : defs) {
        if (i.first == "@") return PrintError("invalid variable name '@'");
        if (env_->GetType(i.first, false) != kTypeError) {
            return PrintError("has already been defined", i.first.c_str());
        }
        auto init_type = i.second->SemaAnalyze(*this);
        // type deduce
        if (type == kVar && !deduced) {
            if (init_type == kVar) {
                return PrintError("cannot deduce the type of a expression with a uncertain type");
            }
            type = init_type;
            deduced = true;
        }
        else if (init_type >= kFuncTypeBase && type == kFunction) {
            // if defined a 'function' type function
            // set to the real type of the function
            type = init_type;
        }
        else if (init_type != kVar && init_type != type) {
            return PrintError("type mismatch when initializing a variable");
        }
        // if defined a non-var variable and init_type is 'var' type
        // the type of variable will be the defined type
        env_->Insert(i.first, type);
    }
    return kVoid;   // variable definition will not return value
}

TypeValue Analyzer::AnalyzeBinExpr(int op, TypeValue l_type, TypeValue r_type) {
    if (!IsBinaryOperator(op)) return PrintError("invalid binary operator");
    if (l_type != r_type) return PrintError("type mismatch between lhs and rhs");
    if (!CheckType(op, l_type)) {
        return PrintError("invalid operand type in binary expression");
    }
    return l_type;
}

TypeValue Analyzer::AnalyzeUnaExpr(int op, TypeValue type) {
    if (op != kSub && IsBinaryOperator(op)) {
        return PrintError("invalid unary operator");
    }
    if (!CheckType(op, type)) {
        return PrintError("invalid operand type in unary expression");
    }
    switch (op) {
        case kConvNum: return kNumber;
        case kConvDec: return kFloat;
        case kConvStr: return kString;
        default: return type;
    }
}

TypeValue Analyzer::AnalyzeCall(const ASTPtr &callee, const ASTPtrList &args) {
    auto ret = callee->SemaAnalyze(*this);
    if (ret == kTypeError) return kTypeError;
    if (ret < kFuncTypeBase && ret != kFunction && ret != kVar) {
        return PrintError("callee is not a function");
    }

    // cannot confirm the return type of type 'function'
    // type 'var' means a kind of uncertain type
    if (ret == kFunction || ret == kVar) return kVar;
    // TODO: call a 'var' type variable may cause system failure

    auto ret_type = GetFuncRetType(ret);
    auto arg_type = (ret - ret_type - kFuncTypeBase) / kFuncTypeBase;

    TypeValue a_type = 0;
    for (const auto &i : args) {
        auto i_type = i->SemaAnalyze(*this);
        if (i_type == kTypeError) return kTypeError;
        if (i_type >= kFuncTypeBase) i_type = kFunction;
        a_type = a_type * kFuncTypeBase + i_type + 1;
    }
    if (a_type != arg_type) return PrintError("invalid function call");

    return ret_type;
}

TypeValue Analyzer::AnalyzeBlock(const ASTPtrList &expr_list) {
    NewEnvironment();
    for (const auto &i : expr_list) {
        auto ret = i->SemaAnalyze(*this);
        if (ret == kTypeError) return kTypeError;
    }
    RestoreEnvironment();
    return kVoid;
}

TypeValue Analyzer::AnalyzeFunc(const ASTPtrList &args, TypeValue ret_type, const ASTPtr &body) {
    NewEnvironment();

    std::vector<TypeValue> args_type;
    for (const auto &i : args) {
        args_type.push_back(i->SemaAnalyze(*this));
    }
    auto func_type = GetFunctionType(args_type, ret_type);
    if (func_type == kTypeError) return PrintError("invalid function definition");
    env_->Insert("@", func_type);   // insert '@' into current environment

    if (body->SemaAnalyze(*this) == kTypeError) return kTypeError;

    RestoreEnvironment();
    return func_type;
}

TypeValue Analyzer::AnalyzeIf(const ASTPtr &cond, const ASTPtr &then, const ASTPtr &else_then) {
    if (cond->SemaAnalyze(*this) == kTypeError) return kTypeError;
    if (then->SemaAnalyze(*this) == kTypeError) return kTypeError;
    if (else_then->SemaAnalyze(*this) == kTypeError) return kTypeError;
    return kVoid;
}

TypeValue Analyzer::AnalyzeWhile(const ASTPtr &cond, const ASTPtr &body) {
    if (cond->SemaAnalyze(*this) == kTypeError) return kTypeError;
    if (body->SemaAnalyze(*this) == kTypeError) return kTypeError;
    return kVoid;
}

TypeValue Analyzer::AnalyzeCtrlFlow(int ctrlflow_type, const ASTPtr &value) {
    if (ctrlflow_type == kReturn) {
        auto ret = env_->GetType("@");
        if (ret != kTypeError) {
            if (value != nullptr) {
                auto ret_type = GetFuncRetType(ret);
                auto r_type = value->SemaAnalyze(*this);
                if (r_type >= kFuncTypeBase) r_type = kFunction;
                if (ret_type != r_type) {
                    return PrintError("type mismatch when return from function");
                }
            }
        }
        else {
            return PrintError("cannot return outside the function");
        }
    }
    return kVoid;
}

TypeValue Analyzer::AnalyzeExtern(int ext_type, const LibList &libs) {
    if (env_->outer() != nullptr) {
        return PrintError("cannot import/export libraries in nested block");
    }
    if (ext_type == kImport) {   // TODO: test
        // load symbol table
        for (const auto &i : libs) {
            if (!env_->LoadEnv((lib_path_ + i + ".saby.sym").c_str())) {
                return PrintError("cannot be imported", i.c_str());
            }
        }
    }
    else { // ext_type == kExport
        // export symbol table
        if (!env_->SaveEnv(sym_path_.c_str(), libs)) {
            return PrintError("cannot export symbol table");
        }
    }
    return kVoid;
}
