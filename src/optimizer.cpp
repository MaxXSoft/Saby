#include "optimizer.h"

#include <limits>
#include <sstream>
#include <cmath>
#include <cstdlib>

#include "lexer.h"

namespace {

const long long kNumberLimit[] = {std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max()};
const double kFloatLimit[] = {std::numeric_limits<double>::min(), std::numeric_limits<double>::max()};

inline long long GetPopCount(long long num_val) {
    // TODO: consider using 'popcnt' (Intel SSE 4.2+)
    num_val = (num_val & 0x5555555555555555) + ((num_val >> 1) & 0x5555555555555555);
    num_val = (num_val & 0x3333333333333333) + ((num_val >> 2) & 0x3333333333333333);
    num_val = (num_val & 0x0f0f0f0f0f0f0f0f) + ((num_val >> 4) & 0x0f0f0f0f0f0f0f0f);
    num_val = (num_val & 0x00ff00ff00ff00ff) + ((num_val >> 8) & 0x00ff00ff00ff00ff);
    num_val = (num_val & 0x0000ffff0000ffff) + ((num_val >> 16) & 0x0000ffff0000ffff);
    num_val = (num_val & 0x00000000ffffffff) + ((num_val >> 32) & 0x00000000ffffffff);
    return num_val;
}

inline long long ConvertToNum(double dec_val) {
    return static_cast<long long>(dec_val);
}

long long ConvertToNum(const std::string &str_val) {
    // we do not care whether the conversion is valid
    return std::strtoll(str_val.c_str(), nullptr, 10);
}

inline double ConvertToDec(long long num_val) {
    return static_cast<double>(num_val);
}

double ConvertToDec(const std::string &str_val) {
    // we do not care whether the conversion is valid
    return std::strtod(str_val.c_str(), nullptr);
}

std::string ConvertToStr(long long num_val) {
    std::stringstream ss;
    ss << num_val;
    return ss.str();
}

std::string ConvertToStr(double dec_val) {
    std::stringstream ss;
    ss << dec_val;
    return ss.str();
}

} // namespace

template <typename T>
T Optimizer::CalcValue(Operator op, const T &lhs, const T &rhs) {
    switch (op) {
        case Operator::And: return static_cast<long long>(lhs) & static_cast<long long>(rhs);
        case Operator::Xor: return static_cast<long long>(lhs) ^ static_cast<long long>(rhs);
        case Operator::Or: return static_cast<long long>(lhs) | static_cast<long long>(rhs);
        case Operator::Shl: return static_cast<long long>(lhs) << static_cast<long long>(rhs);
        case Operator::Shr: return static_cast<long long>(lhs) >> static_cast<long long>(rhs);
        case Operator::Add: return lhs + rhs;
        case Operator::Sub: return lhs - rhs;
        case Operator::Mul: return lhs * rhs;
        case Operator::Div: return lhs / rhs;
        case Operator::Mod: return static_cast<long long>(lhs) % static_cast<long long>(rhs);
        case Operator::Pow: return pow(lhs, rhs);
        case Operator::Less: return lhs < rhs;
        case Operator::LessEqual: return lhs <= rhs;
        case Operator::Greater: return lhs > rhs;
        case Operator::GreaterEqual: return lhs >= rhs;
        case Operator::Equal: return lhs == rhs;
        case Operator::NotEqual: return lhs != rhs;
        default: return 0;
    }
}

SSAPtr Optimizer::ConstFold(Operator op, const SSAPtr &lhs, const SSAPtr &rhs) {
    // both of lhs or rhs must be constant
    if (!IsSSAType<ValueSSA>(lhs) || !IsSSAType<ValueSSA>(rhs)) return nullptr;
    SSAPtr value;
    auto lhs_ssa = SSACast<ValueSSA>(lhs);
    auto rhs_ssa = SSACast<ValueSSA>(rhs);
    switch (lhs->name()[1]) {
        case 'n': {   // #num: Number
            auto lhs_v = lhs_ssa->num_val();
            auto rhs_v = rhs_ssa->num_val();
            value = std::make_shared<ValueSSA>(CalcValue<long long>(op, lhs_v, rhs_v));
            break;
        }
        case 'd': {   // #dec: Decimal
            auto lhs_v = lhs_ssa->dec_val();
            auto rhs_v = rhs_ssa->dec_val();
            value = std::make_shared<ValueSSA>(CalcValue<double>(op, lhs_v, rhs_v));
            break;
        }
        case 's': {   // #str: String
            auto lhs_v = lhs_ssa->str_val();
            auto rhs_v = rhs_ssa->str_val();
            switch (op) {
                case Operator::Add: {   // string catenate
                    value = std::make_shared<ValueSSA>(lhs_v + rhs_v);
                    break;
                }
                case Operator::Equal: {   // str1 == str2, returns a number
                    value = std::make_shared<ValueSSA>(static_cast<long long>(lhs_v == rhs_v));
                    break;
                }
                case Operator::NotEqual: {   // str1 != str2, returns a number
                    value = std::make_shared<ValueSSA>(static_cast<long long>(lhs_v != rhs_v));
                    break;
                }
                default:;
            }
            break;
        }
    }
    return value;
}

SSAPtr Optimizer::ConstFoldUna(Operator op, const SSAPtr &operand) {
    // 'op' can be: ConvNum, ConvDec, ConvStr, Not
    if (!IsSSAType<ValueSSA>(operand)) return nullptr;
    auto opr_ssa = SSACast<ValueSSA>(operand);
    switch (op) {
        case Operator::ConvNum: {
            if (opr_ssa->name()[1] == 'd') {
                return std::make_shared<ValueSSA>(ConvertToNum(opr_ssa->dec_val()));
            }
            else {   // name()[0] == 's'
                return std::make_shared<ValueSSA>(ConvertToNum(opr_ssa->str_val()));
            }
            break;
        }
        case Operator::ConvDec: {
            if (opr_ssa->name()[1] == 'n') {
                return std::make_shared<ValueSSA>(ConvertToDec(opr_ssa->num_val()));
            }
            else {   // name()[0] == 's'
                return std::make_shared<ValueSSA>(ConvertToDec(opr_ssa->str_val()));
            }
            break;
        }
        case Operator::ConvStr: {
            if (opr_ssa->name()[1] == 'n') {
                return std::make_shared<ValueSSA>(ConvertToStr(opr_ssa->num_val()));
            }
            else {   // name()[0] == 'd'
                return std::make_shared<ValueSSA>(ConvertToStr(opr_ssa->dec_val()));
            }
            break;
        }
        case Operator::Not: {
            return std::make_shared<ValueSSA>(~opr_ssa->num_val());
            break;
        }
        default:;
    }
    return nullptr;
}

// TODO: need to be optimized
SSAPtr Optimizer::AlgebraSimplify(Operator op, const SSAPtr &lhs, const SSAPtr &rhs, int type) {
    SSAPtr value = nullptr;        // non-constant value
    ValueSSA *k_value = nullptr;   // constant value
    bool equal, is_lhs_const;      // lhs == rhs, lhs is constant value
    if (lhs == rhs) {
        // handle Equal & NotEqual when lhs == rhs
        switch (op) {
            case Operator::Equal: return std::make_shared<ValueSSA>(1LL);
            case Operator::NotEqual: return std::make_shared<ValueSSA>(0LL);
            default: equal = true;
        }
    }
    else {
        switch (op) {
            case Operator::Equal: return std::make_shared<ValueSSA>(0LL);
            case Operator::NotEqual: return std::make_shared<ValueSSA>(1LL);
            default: {
                // one of operand is a constant
                if (IsSSAType<ValueSSA>(lhs)) {
                    k_value = SSACast<ValueSSA>(lhs);
                    value = rhs;
                    equal = false;
                    is_lhs_const = true;
                }
                else if (IsSSAType<ValueSSA>(rhs)) {
                    k_value = SSACast<ValueSSA>(rhs);
                    value = lhs;
                    equal = false;
                    is_lhs_const = false;
                }
                else {
                    // cannot do algebraic simplification
                    return nullptr;
                }
            }
        }
    }
    // pattern of optimizing logic expression
    auto OptimizeLogicExpression = [&equal, &is_lhs_const, &k_value, &type](long long num_val, bool is_lhs_max) {
        const auto lhs_index = is_lhs_max ? 1 : 0;
        const auto rhs_index = is_lhs_max ? 0 : 1;
        if (equal) {
            return std::make_shared<ValueSSA>(num_val);
        }
        else if (is_lhs_const) {   // e.g. V_MIN <= v = 1
            if ((type == kNumber && k_value->num_val() == kNumberLimit[lhs_index])
                    || (type == kFloat && k_value->dec_val() == kFloatLimit[lhs_index])) {
                return std::make_shared<ValueSSA>(num_val);
            }
        }
        else {   // e.g. v <= V_MAX = 1
            if ((type == kNumber && k_value->num_val() == kNumberLimit[rhs_index])
                    || (type == kFloat && k_value->dec_val() == kFloatLimit[rhs_index])) {
                return std::make_shared<ValueSSA>(num_val);
            }
        }
        return std::shared_ptr<ValueSSA>(nullptr);
    };
    // main process of algebraic simplification
    switch (op) {
        case Operator::And: {   // v & v = v; v & 0 = 0; v & V_UMAX = v
            if (equal) {
                return value;
            }
            else {
                auto k_num = k_value->num_val();
                if (k_num == 0) {
                    return std::make_shared<ValueSSA>(0LL);
                }
                else if (k_num == -1) {
                    return value;
                }
            }
            break;
        }
        case Operator::Xor: {   // v ^ v = 0; v ^ 0 = v
            if (equal) {
                return std::make_shared<ValueSSA>(0LL);
            }
            else if (k_value->num_val() == 0) {
                return value;
            }
            break;
        }
        case Operator::Or: {   // v | v = v; v | 0 = v; v | V_UMAX = V_UMAX
            if (equal) {
                return value;
            }
            else {
                auto k_num = k_value->num_val();
                if (k_num == 0) {
                    return value;
                }
                else if (k_num == -1) {
                    return std::make_shared<ValueSSA>(-1LL);
                }
            }
            break;
        }
        case Operator::Shl: case Operator::Shr: {
            // v << 0 = v; v >> 0 = v; 0 << v = 0; 0 >> v = 0; -1 >> v = -1
            if (!equal) {
                if (k_value->num_val() == 0) {
                    return is_lhs_const ? std::make_shared<ValueSSA>(0LL) : value;
                }
                if (op == Operator::Shr && is_lhs_const && k_value->num_val() == -1LL) {
                    return std::make_shared<ValueSSA>(-1LL);
                }
            }
            break;
        }
        case Operator::Add: {   // v + 0 = v; v + "" = v
            if (!equal) {
                if (type == kNumber && k_value->num_val() == 0) return value;
                if (type == kFloat && k_value->dec_val() == .0) return value;
                if (type == kString && k_value->str_val().empty()) return value;
            }
            break;
        }
        case Operator::Sub: {   // v - v = 0; v - 0 = v
            if (equal) {
                return type == kNumber ?
                        std::make_shared<ValueSSA>(0LL) :
                        std::make_shared<ValueSSA>(0.);
            }
            else if (!is_lhs_const) {
                if (type == kNumber && k_value->num_val() == 0) return value;
                if (type == kFloat && k_value->dec_val() == 0.) return value;
            }
            break;
        }
        case Operator::Mul: {   // v * 0 = 0; v * 1 = v
            if (!equal) {
                if (type == kNumber && k_value->num_val() == 0) {
                    return std::make_shared<ValueSSA>(0LL);
                }
                else if (type == kFloat && k_value->dec_val() == 0.) {
                    return std::make_shared<ValueSSA>(0.);
                }
                else if ((type == kNumber && k_value->num_val() == 1)
                        || (type == kFloat && k_value->num_val() == 1.)) {
                    return value;
                }
            }
            break;
        }
        case Operator::Div: {   // v / v = 1; 0 / v = 0; v / 1 = v
            if (equal) {
                return type == kNumber ?
                        std::make_shared<ValueSSA>(1LL) :
                        std::make_shared<ValueSSA>(1.);
            }
            else if (is_lhs_const) {   // 0 / v = 0
                if (type == kNumber && k_value->num_val() == 0) return std::make_shared<ValueSSA>(0LL);
                if (type == kFloat && k_value->dec_val() == 0.) return std::make_shared<ValueSSA>(0.);
            }
            else {   // v / 1 = v
                if ((type == kNumber && k_value->num_val() == 1)
                        || (type == kFloat && k_value->num_val() == 1.)) {
                    return value;
                }
            }
            break;
        }
        case Operator::Mod: {   // v % v = 0; 0 % v = 0; v % 1 = 0
            if (equal) {
                return type == kNumber ?
                        std::make_shared<ValueSSA>(0LL) :
                        std::make_shared<ValueSSA>(0.);
            }
            else if (is_lhs_const) {   // 0 % v = 0
                if (type == kNumber && k_value->num_val() == 0) return std::make_shared<ValueSSA>(0LL);
                if (type == kFloat && k_value->dec_val() == 0.) return std::make_shared<ValueSSA>(0.);
            }
            else {   // v % 1 = 0
                if (type == kNumber && k_value->num_val() == 1) return std::make_shared<ValueSSA>(0LL);
                if (type == kFloat && k_value->dec_val() == 1.) return std::make_shared<ValueSSA>(0.);
            }
            break;
        }
        case Operator::Pow: {   // v ** 0 = 1; 0 ** v = 0; v ** 1 = v; 1 ** v = 1
            // NOTE, TODO: 0 ** 0 may return 1 or 0
            if (!equal) {
                if (is_lhs_const) {   // 0 ** v = 0; 1 ** v = 1
                    if (k_value->dec_val() == 0.) return std::make_shared<ValueSSA>(0.);
                    if (k_value->dec_val() == 1.) return std::make_shared<ValueSSA>(1.);
                }
                else {   // v ** 0 = 1; v ** 1 = v
                    if (k_value->dec_val() == 0.) return std::make_shared<ValueSSA>(1.);
                    if (k_value->dec_val() == 1.) return value;
                }
            }
            break;
        }
        case Operator::Less: {   // v < v = 0; v < V_MIN = 0; V_MAX < v = 0
            return OptimizeLogicExpression(0, true);
        }
        case Operator::LessEqual: {   // v <= v = 1; v <= V_MAX = 1; V_MIN <= v = 1
            return OptimizeLogicExpression(1, false);
        }
        case Operator::Greater: {   // v > v = 0; v > V_MAX = 0; V_MIN > v = 0
            return OptimizeLogicExpression(0, false);
        }
        case Operator::GreaterEqual: {   // v >= v = 1; v >= V_MIN = 1; V_MAX >= v = 1
            return OptimizeLogicExpression(1, true);
        }
        default:;
    }
    // cannot do optimization, return a null pointer
    return nullptr;
}

SSAPtr Optimizer::StrengthReduct(Operator op, const SSAPtr &lhs, const SSAPtr &rhs, int type) {
    // NOTE: this process will not do any optimization about loop
    //       it can only do some naive optimizations
    switch (op) {
        case Operator::Add: {   // v + v = v << 1 (Number type)
            if (lhs == rhs && type == kNumber) {
                auto value = std::make_shared<ValueSSA>(2LL);
                return std::make_shared<QuadSSA>(Operator::Shl, lhs, value);
            }
            break;
        }
        case Operator::Mul: {   // v * (2 ^ n) = v << n (Number type)
            if (type == kNumber) {
                SSAPtr value;
                long long num_val;
                if (IsSSAType<ValueSSA>(lhs)) {
                    num_val = SSACast<ValueSSA>(lhs)->num_val();
                    value = rhs;
                }
                else if (IsSSAType<ValueSSA>(rhs)) {
                    num_val = SSACast<ValueSSA>(rhs)->num_val();
                    value = lhs;
                }
                else {
                    return nullptr;
                }
                // check if num_val is power of 2 (num_val != 0)
                if ((num_val & (num_val - 1)) == 0) {
                    auto num_ssa = std::make_shared<ValueSSA>(GetPopCount(num_val - 1));
                    return std::make_shared<QuadSSA>(Operator::Shl, value, num_ssa);
                }
            }
            break;
        }
        case Operator::Div: {   // v / (2 ^ n) = v >> n (Number type)
            if (type == kNumber && IsSSAType<ValueSSA>(rhs)) {
                auto num_val = SSACast<ValueSSA>(rhs)->num_val();
                if ((num_val & (num_val - 1)) == 0) {
                    auto num_ssa = std::make_shared<ValueSSA>(GetPopCount(num_val - 1));
                    return std::make_shared<QuadSSA>(Operator::Shr, lhs, num_ssa);
                }
            }
            break;
        }
        // NOTE: for ZexVM, we cannot use this kind of optimization
        //       because it generates multiple instructions,
        //       compared with single instruction, it slows down the program
        case Operator::Mod: {
            // v % (2 ^ n) = v & (2 ^ n - 1)                   (positive num)
            // v % (2 ^ n) = (v & (2 ^ n - 1)) | ~(2 ^ n - 1)  (negative num)
            break;
        }
        case Operator::Pow: {   // 2 ** v = 1 << v
            // need to generate Float type value, ignore, same as above
            break;
        }
        default:;
    }
    return nullptr;
}

SSAPtr Optimizer::CopyProp(const SSAPtr &rhs) {
    if (IsSSAType<VariableSSA>(rhs)) {
        // get the value of variable
        auto var = SSACast<VariableSSA>(rhs);
        const auto &value = (*var)[0].value();
        if (IsSSAType<VariableSSA>(value)) {
            auto ret = CopyProp(value);
            return ret ? ret : value;
        }
        else if (IsSSAType<ValueSSA>(value) || IsSSAType<ArgGetterSSA>(value)
                || IsSSAType<ExternFuncSSA>(value) || IsSSAType<BlockSSA>(value)) {
            // value that can be propagated
            return value;
        }
        else {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
}

// public method
SSAPtr Optimizer::OptimizeBinExpr(Operator op, SSAPtr &lhs, SSAPtr &rhs, int type) {
    if (!enabled_) return nullptr;
    // auto copy propagation
    auto lhs_new = CopyProp(lhs);
    auto rhs_new = CopyProp(rhs);
    if (lhs_new) lhs = lhs_new;
    if (rhs_new) rhs = rhs_new;
    // constant folding
    auto cf_result = ConstFold(op, lhs, rhs);
    if (cf_result) return cf_result;
    // algebraic simplification
    auto as_result = AlgebraSimplify(op, lhs, rhs, type);
    if (as_result) return as_result;
    // strength reduction
    return StrengthReduct(op, lhs, rhs, type);
}

SSAPtr Optimizer::OptimizeUnaExpr(Operator op, SSAPtr &operand) {
    if (!enabled_) return nullptr;
    // auto copy propagation
    auto opr_new = CopyProp(operand);
    if (opr_new) operand = opr_new;
    // constant folding
    return ConstFoldUna(op, operand);
}

SSAPtr Optimizer::OptimizeAssign(const SSAPtr &rhs) {
    return enabled_ ? CopyProp(rhs) : nullptr;
}

