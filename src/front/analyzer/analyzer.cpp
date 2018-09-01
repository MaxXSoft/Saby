// Semantic Analyzer

#include "analyzer.h"

#include <cstdio>
#include <string>
#include <vector>

#include "../../define/type.h"
#include "../../util/fs/dir.h"

namespace {

TypeValue GetFunctionType(const TypeList &args_type, TypeValue ret_type) {
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
        case kNot: case kInc: case kDec: {
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
                lexer_.line_pos(), id, description);
    }
    else {
        fprintf(stderr, "\033[1manalyzer\033[0m(before line %u): "
                        "\033[31m\033[1merror:\033[0m %s\n", 
                lexer_.line_pos(), description);
    }
    ++error_num_;
    return kTypeError;
}

void Analyzer::PrintWarning(const char *description, const char *id) {
    fprintf(stderr, "\033[1manalyzer\033[0m(before line %u): "
                    "\033[35m\033[1mwarning:\033[0m id '%s' %s\n", 
            lexer_.line_pos(), id, description);
    ++warning_num_;
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

TypeValue Analyzer::AnalyzeVar(const VarTypeList &defs, TypeValue type) {
    auto deduced = false;   // whether type has been deduced
    for (const auto &i : defs) {
        if (i.first == "@") return PrintError("invalid variable name '@'");
        if (env_->GetType(i.first, false) != kTypeError) {
            return PrintError("has already been defined", i.first.c_str());
        }
        auto init_type = i.second;
        if (init_type == kTypeError) return kTypeError;
        // type deduce
        if (type == kVar && !deduced) {
            if (init_type == kVar || init_type == kVoid) {
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

TypeValue Analyzer::AnalyzeBinExpr(int op, TypeValue l_type, TypeValue r_type, bool is_lvalue) {
    if (!IsBinaryOperator(op)) return PrintError("invalid binary operator");
    if (r_type == kVar && op == kAssign) {
        r_type = l_type;   // implicit conversion of uncertain type
        // NOTE: it's convenient, but unsafe
    }
    else if (l_type != r_type) {
        return PrintError("type mismatch between lhs and rhs");
    }
    if (!CheckType(op, l_type)) {
        return PrintError("invalid operand type in binary expression");
    }
    // 'op' is assignment operator
    if (op >= kAssign && !is_lvalue) {
        return PrintError("assignment operator must be applied to lvalue");
    }
    switch (op) {
        case kLess: case kLesEql: case kGreat:
        case kGreEql: case kEql: case kNeq: return kNumber;
        default: return l_type;
    }
}

TypeValue Analyzer::AnalyzeUnaExpr(int op, TypeValue type, bool is_lvalue) {
    if (op != kSub && IsBinaryOperator(op)) {
        return PrintError("invalid unary operator");
    }
    if (!CheckType(op, type)) {
        return PrintError("invalid operand type in unary expression");
    }
    // 'op' is inc/dec operator
    if ((op == kInc || op == kDec) && !is_lvalue) {
        return PrintError("inc/dec operator must be applied to lvalue");
    }
    switch (op) {
        case kConvNum: return kNumber;
        case kConvDec: return kFloat;
        case kConvStr: return kString;
        default: return type;
    }
}

TypeValue Analyzer::AnalyzeCall(TypeValue callee, const TypeList &args) {
    if (callee == kTypeError) return kTypeError;
    if (callee < kFuncTypeBase && callee != kFunction && callee != kVar) {
        return PrintError("callee is not a function");
    }

    // cannot confirm the return type of type 'function'
    // type 'var' means a kind of uncertain type
    if (callee == kFunction || callee == kVar) return kVar;
    // TODO: call a 'var' type variable may cause system failure

    auto ret_type = GetFuncRetType(callee);
    auto arg_type = (callee - ret_type - kFuncTypeBase) / kFuncTypeBase;

    TypeValue a_type = 0;
    for (const auto &i : args) {
        auto i_type = i;
        if (i_type == kTypeError) return kTypeError;
        if (i_type >= kFuncTypeBase) i_type = kFunction;
        a_type = a_type * kFuncTypeBase + i_type + 1;
    }
    if (a_type != arg_type) return PrintError("invalid function call");

    return ret_type;
}

TypeValue Analyzer::AnalyzeFunc(const TypeList &args, TypeValue ret_type) {
    if (args.size() > kFuncMaxArgNum) {
        return PrintError("the number of arguments exceeds the limit");
    }
    auto func_type = GetFunctionType(args, ret_type);
    if (func_type == kTypeError) return PrintError("invalid function definition");
    env_->Insert("@", func_type);   // insert '@' into current environment
    return func_type;
}

TypeValue Analyzer::AnalyzeFuncReturn(TypeValue return_type) {
    if (return_type != kVoid && !has_return_) {
        return PrintError("non-void function must have a return value");
    }
    else {
        return kVoid;
    }
}

TypeValue Analyzer::AnalyzeCtrlFlow(int ctrlflow_type, TypeValue value) {
    if (ctrlflow_type == kReturn) {
        has_return_ = true;
        auto ret = env_->GetType("@");
        if (ret != kTypeError) {
            auto ret_type = GetFuncRetType(ret);
            // kTypeError means returning 'void'
            if (value == kTypeError) {
                value = kVoid;
            }
            else if (value >= kFuncTypeBase) {
                value = kFunction;
            }
            if (ret_type != value) {
                return PrintError("type mismatch when return from function");
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
    if (ext_type == kImport) {
        // load symbol table
        for (const auto &i : libs) {
            auto cur_lib = GetRealPath(lib_path_ + i + ".saby.sym");
            if (cur_lib == sym_path_) {
                PrintWarning("was skipped, self-importing "
                             "is not allowed", i.c_str());
                continue;
            }
            using LEReturn = Environment::LoadEnvReturn;
            switch (env_->LoadEnv(cur_lib.c_str(), i)) {
                case LEReturn::FileError: {
                    return PrintError("cannot be imported", i.c_str());
                }
                case LEReturn::LibConflicted: {
                    PrintWarning("has already been imported", i.c_str());
                    break;
                }
                case LEReturn::FuncConflicted: {
                    PrintWarning("has some functions conflicts "
                                 "with existing id", i.c_str());
                    break;
                }
                default:;
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
