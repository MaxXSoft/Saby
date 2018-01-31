#ifndef SABY_SSA_H_
#define SABY_SSA_H_

#include <memory>
#include <utility>
#include <list>
#include <string>
#include <cstddef>

#include "type.h"
#include "def_use.h"
// #include "lexer.h"   // 'Operator' in QuadSSA

class ValueSSA : public Value {
public:
    ValueSSA(long long value)
            : Value("#num"), num_val_(value), type_(ValueType::Number) {}
    ValueSSA(double value)
            : Value("#dec"), dec_val_(value), type_(ValueType::Decimal) {}
    ValueSSA(const std::string &value)
            : Value("#str"), str_val_(value), type_(ValueType::String) {}

    // SSAPtr Duplicate() {
    //     switch (type_) {
    //         case ValueType::Number: return std::make_shared<ValueSSA>(num_val_);
    //         case ValueType::Decimal: return std::make_shared<ValueSSA>(dec_val_);
    //         case ValueType::String: return std::make_shared<ValueSSA>(str_val_);
    //     }
    // }

    void Print(int indent) override;

private:
    enum class ValueType : char {
        Number, Decimal, String
    };
    ValueType type_;
    long long num_val_;
    double dec_val_;
    std::string str_val_;
};

// used to represent arguments in the body of a function
class ArgHolderSSA : public Value {
public:
    ArgHolderSSA(IDType arg_id) : Value("#arg"), arg_id_(arg_id) {}

    void Print(int indent) override;

private:
    IDType arg_id_;
};

// store inline assembly block
// nothing can use it although it's a value
class AsmSSA : public Value {
public:
    AsmSSA(const std::string &text) : Value("asm:"), text_(text) {}

    void Print(int indent) override;

private:
    std::string text_;
};

class UndefSSA : public Value {
public:
    UndefSSA() : Value("#und") {}

    void Print(int indent) override;
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

    void Print(int indent) override;

    void set_ref(const std::shared_ptr<PhiSSA> &phi) { ref_ = phi; }
    SSAPtr ref() { return ref_.lock(); }

    IDType block_id() const { return block_id_; }

private:
    IDType block_id_;
    std::weak_ptr<PhiSSA> ref_;
};

class BlockSSA : public User {
public:
    BlockSSA(IDType id)
            : User("block:"), id_(id), is_func_(false) {}

    void AddPred(SSAPtr pred) { preds_.push_back(pred); }
    void AddValue(SSAPtr value) { push_back(Use(value, this)); }

    void Print(int indent) override;

    void set_is_func(bool is_func) { is_func_ = is_func; }

    IDType id() const { return id_; }
    bool is_func() const { return is_func_; }
    const std::list<SSAPtr> &preds() const { return preds_; }

private:
    IDType id_;
    bool is_func_;
    std::list<SSAPtr> preds_;
};

class JumpSSA : public User {
public:
    JumpSSA(std::shared_ptr<BlockSSA> block, SSAPtr cond) : User("jump->") {
        if (cond) {            
            reserve(2);
            push_back(Use(block, this));
            push_back(Use(cond, this));
        }
        else {
            reserve(1);
            push_back(Use(block, this));
        }
    }

    void Print(int indent) override;
};

class CallSSA : public User {
public:
    CallSSA(std::shared_ptr<BlockSSA> block)
            : User("call") {
        reserve(kFuncMaxArgNum + 1);
        push_back(Use(block, this));
    }

    void AddArgument(SSAPtr value) {
        if (size() <= kFuncMaxArgNum + 1) push_back(Use(value, this));
    }

    void Print(int indent) override;
};

class QuadSSA : public User {
public:
    enum class Operator : char {
        ConvNum, ConvDec, ConvStr,
        And, Xor, Or, Not, Shl, Shr,
        Add, Sub, Mul, Div, Mod, Pow,
        Less, LessEqual, Greater, GreaterEqual, Euqal, NotEqual,
        Return
    };

    QuadSSA(Operator op, SSAPtr opr1, SSAPtr opr2) : User("inst"), op_(op) {
        if (opr2) {
            reserve(2);
            push_back(Use(opr1, this));
            push_back(Use(opr2, this));
        }
        else {
            reserve(1);
            push_back(Use(opr1, this));
        }
    }

    void Print(int indent) override;

private:
    Operator op_;   // TODO: rewrite Operator enum
};

class VariableSSA : public User {
public:
    VariableSSA(IDType id, SSAPtr value) : User("$var"), id_(id) {
        if (value) {
            reserve(1);
            push_back(Use(value, this));
        }
        else {
            reserve(0);
        }
    }

    void Print(int indent) override;

    IDType id() const { return id_; }

private:
    IDType id_;
};

#endif // SABY_SSA_H_
