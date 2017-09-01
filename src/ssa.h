#ifndef SABY_SSA_H_
#define SABY_SSA_H_

#include <memory>
#include <vector>
#include <string>

#include "lexer.h"

class BaseSSA {
public:
    virtual ~BaseSSA() = default;
};

using SSAPtr = std::unique_ptr<BaseSSA>;
using SSAPtrList = std::vector<SSAPtr>;

class BlockSSA : public BaseSSA {
public:
    BlockSSA(SSAPtr prev, SSAPtr next, SSAPtrList body)
            : prev_(std::move(prev)), next_(std::move(next)),
              body_(std::move(body)) {}

private:
    SSAPtr prev_, next_;
    SSAPtrList body_;
};

class ValueSSA : public BaseSSA {
public:
    ValueSSA(long long value) : num_val_(value), type_(ValueType::Number) {}
    ValueSSA(double value) : dec_val_(value), type_(ValueType::Decimal) {}
    ValueSSA(const std::string &value) : str_val_(value), type_(ValueType::String) {}

private:
    enum class ValueType : char {
        Number, Decimal, String
    };
    ValueType type_;
    long long num_val_;
    double dec_val_;
    std::string str_val_;
};

class VariableSSA : public BaseSSA {
public:
    VariableSSA(int id) : id_(id) {}

private:
    int id_;
};

class PhiSSA : public BaseSSA {
public:
    PhiSSA(SSAPtrList opr_list) : opr_list_(std::move(opr_list)) {}

    const SSAPtrList &users() const { return users_; }
    void AddUser(SSAPtr user) { users_.push_back(std::move(user)); }

private:
    SSAPtrList opr_list_, users_;
};

class QuadSSA : public BaseSSA {
public:
    QuadSSA(SSAPtr dst, Operator op, SSAPtr opr1, SSAPtr opr2)
            : dst_(std::move(dst)), op_(op), opr1_(std::move(opr1)), opr2_(std::move(opr2)) {}

private:
    SSAPtr dst_, opr1_, opr2_;
    Operator op_;
};

class JumpSSA : public BaseSSA {
public:
    JumpSSA(SSAPtr block) : block_(std::move(block)) {}

private:
    SSAPtr block_;
};

#endif // SABY_SSA_H_
