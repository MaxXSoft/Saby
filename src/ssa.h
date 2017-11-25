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
    // virtual void Destroy() = 0;   // destroy all of the objects
};

using SSARef = BaseSSA *;
using SSAPtr = std::shared_ptr<BaseSSA>;
using SSAPtrList = std::vector<SSAPtr>;

class BlockSSA : public BaseSSA {
public:
    BlockSSA(IDType id, SSAPtrList body) : id_(id), body_(std::move(body)) {}

    void set_next(SSAPtr next) { next_ = next; }
    void AddPred(SSAPtr pred) { preds_.push_back(pred); }
    const SSAPtrList &preds() const { return preds_; }
    IDType id() const { return id_; }

private:
    IDType id_;
    SSAPtrList preds_, body_;
    SSAPtr next_;
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

class VariableSSA : public BaseSSA {
public:
    VariableSSA(IDType id) : id_(id) {}

private:
    IDType id_;
};

class PhiSSA : public BaseSSA {
public:
    PhiSSA(IDType block_id) : block_id_(block_id) {}
    // PhiSSA(SSAPtrList opr_list) : opr_list_(std::move(opr_list)) {}
    
    void AddOpr(SSAPtr opr) { opr_list_.push_back(opr); }
    void AddUser(SSAPtr user) { users_.push_back(user); }
    void ReplaceBy(SSAPtr &ssa) {
        // TODO
    }

    const SSAPtrList &users() const { return users_; }
    const SSAPtrList &operands() const { return opr_list_; }
    IDType block_id() const { return block_id_; }

private:
    IDType block_id_;
    SSAPtrList opr_list_, users_;
};

class QuadSSA : public BaseSSA {
public:
    QuadSSA(SSAPtr dest, Operator op, SSAPtr opr1, SSAPtr opr2)
            : dest_(dest), op_(op),
              opr1_(opr1), opr2_(opr2) {}

private:
    SSAPtr dest_, opr1_, opr2_;
    Operator op_;
};

class JumpSSA : public BaseSSA {
public:
    JumpSSA(SSAPtr block) : block_(block) {}

private:
    SSAPtr block_;
};

class UndefSSA : public BaseSSA {
public:
    UndefSSA() {}
};

#endif // SABY_SSA_H_
