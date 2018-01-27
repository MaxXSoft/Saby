#ifndef SABY_SSA_H_
#define SABY_SSA_H_

#include <memory>
#include <utility>
#include <list>
#include <string>
#include <cstddef>

#include "def_use.h"
#include "lexer.h"

using IDType = std::size_t;

class ValueSSA : public Value {
public:
    ValueSSA(long long value)
            : Value("#num"), num_val_(value), type_(ValueType::Number) {}
    ValueSSA(double value)
            : Value("#dec"), dec_val_(value), type_(ValueType::Decimal) {}
    ValueSSA(const std::string &value)
            : Value("#str"), str_val_(value), type_(ValueType::String) {}

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

class UndefSSA : public Value {
public:
    UndefSSA() : Value("#und") {}
};

class PhiSSA : public User {
public:
    PhiSSA(IDType block_id) : User("phi"), block_id_(block_id) {}

    void AddOperand(SSAPtr opr) { push_back(Use(opr, this)); }
    void ReplaceBy(SSAPtr &ssa) {
        auto value = dynamic_cast<Value *>(this);
        for (auto &&use : *value) {
            use->set_value(ssa);   // TODO: test
        }
    }

    IDType block_id() const { return block_id_; }

    void set_ref(const std::shared_ptr<PhiSSA> &phi) { ref_ = phi; }
    const SSAPtr &ref() const { return ref_.lock(); }

private:
    IDType block_id_;
    std::weak_ptr<PhiSSA> ref_;
};

class BlockSSA : public User {
public:
    BlockSSA(IDType id)
            : User("block:"), id_(id) {}

    void AddPred(SSAPtr pred) { preds_.push_back(pred); }
    void AddValue(SSAPtr value) { push_back(Use(value, this)); }

    IDType id() const { return id_; }
    const std::list<SSAPtr> &preds() const { return preds_; }

private:
    IDType id_;
    std::list<SSAPtr> preds_;
};

class JumpSSA : public User {
public:
    JumpSSA(std::shared_ptr<BlockSSA> block) : User("jump->") {
        reserve(1);
        push_back(Use(block, this));
    }
};

class QuadSSA : public User {
public:
    QuadSSA(Operator op, SSAPtr opr1, SSAPtr opr2) : User("inst"), op_(op) {
        reserve(2);
        push_back(Use(opr1, this));
        push_back(Use(opr2, this));
    }

private:
    Operator op_;
};

class VariableSSA : public User {
public:
    VariableSSA(IDType id, SSAPtr value) : User("$var") {
        reserve(1);
        push_back(Use(value, this));
    }

    IDType id() const { return id_; }

private:
    IDType id_;
};

#endif // SABY_SSA_H_
