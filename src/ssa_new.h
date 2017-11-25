#ifndef SABY_SSA_H_
#define SABY_SSA_H_

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>

#include "lexer.h"   // QuadSSA -> Operator

using IDType = std::size_t;

class BaseSSA {
public:
    virtual ~BaseSSA() = default;
    // virtual void Generate() = 0;

    void set_next(std::shared_ptr<BaseSSA> next) { next_ = next; }

protected:
    std::shared_ptr<BaseSSA> next_;
};

using SSARef = BaseSSA *;
using SSAPtr = std::shared_ptr<BaseSSA>;
using SSAPtrList = std::vector<SSAPtr>;

class UseSSA : public BaseSSA {
public:
    void ReplaceBy(SSAPtr &ssa) { ref_ = ssa; }

protected:
    UseSSA(SSAPtr ref) : ref_(ref) {}

    SSAPtr ref_;
};

using UsePtr = std::shared_ptr<UseSSA>;

class DefSSA : public BaseSSA {
protected:
    DefSSA(IDType id) : id_(id) {}

    IDType id_;
    SSAPtrList users_;
};

class PhiSSA : public BaseSSA {
public:
    void AddOpr(SSAPtr opr) { opr_list_.push_back(opr); }
    void AddUser(SSAPtr user) { users_.push_back(user); }
    void ReplaceBy(SSAPtr &ssa) {
        // TODO
    }

    const SSAPtrList &users() const { return users_; }
    const SSAPtrList &operands() const { return opr_list_; }

private:
    SSAPtrList opr_list_, users_;
};

class ValueSSA : public BaseSSA {
public:
    ValueSSA(long long value) : num_val_(value), type_(ValueType::Number) {}
    ValueSSA(double value) : dec_val_(value), type_(ValueType::Decimal) {}
    ValueSSA(const std::string &value) : str_val_(value), type_(ValueType::String) {}

    SSAPtr Duplicate() {
        switch (type_) {
            case ValueType::Number: return std::make_shared<ValueSSA>(num_val_);
            case ValueType::Decimal: return std::make_shared<ValueSSA>(dec_val_);
            case ValueType::String: return std::make_shared<ValueSSA>(str_val_);
        }
    }

private:
    enum class ValueType : char {
        Number, Decimal, String
    };
    ValueType type_;
    long long num_val_;
    double dec_val_;
    std::string str_val_;
};

class UndefSSA : public BaseSSA {
public:
    UndefSSA() {}
};

class BlockSSA : public DefSSA {
public:
    BlockSSA(IDType id, SSAPtr body) : DefSSA(id), body_(body) {}

    IDType id() const { return id_; }

private:
    SSAPtr body_;
    // SSAPtrList pred_;
};

class JumpSSA : public UseSSA {
public:
    JumpSSA(std::shared_ptr<BlockSSA> block) : UseSSA(block) {}
};

class QuadSSA : public UseSSA {
public:
    QuadSSA(Operator op, UsePtr opr1, UsePtr opr2)
            : UseSSA(opr1), op_(op), opr1_(opr1), opr2_(opr2) {}

private:
    UsePtr opr1_, opr2_;
    Operator op_;
};

#endif // SABY_SSA_H_
