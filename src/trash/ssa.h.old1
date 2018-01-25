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

template <typename T>
inline T *SSACast(const SSAPtr &ptr) {
    return dynamic_cast<T *>(ptr.get());
}

class UseSSA : public BaseSSA {
public:
    void set_ref(SSAPtr ref) { ref_ = ref; }
    const SSAPtr &ref() const { return ref_; }

protected:
    UseSSA(SSAPtr ref) : ref_(ref) {}

    SSAPtr ref_;
};

using UsePtr = std::shared_ptr<UseSSA>;

class DefSSA : public BaseSSA {
public:
    void AddUser(SSAPtr user) { users_.push_back(user); }
    const SSAPtrList &users() const { return users_; }

protected:
    DefSSA(IDType id) : id_(id) {}

    IDType id_;
    SSAPtrList users_;
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

// TODO: rewrite PhiSSA, and redesign use-def chain in a elegant way
class PhiSSA : public DefSSA {
public:
    PhiSSA(IDType block_id) : DefSSA(block_id) {}

    void AddOpr(SSAPtr opr) {
        opr_list_.push_back(opr);
        auto def = SSACast<DefSSA>(opr);
        if (def) def->AddUser(SSAPtr(this));   // unsafe usage
    }

    void ReplaceBy(SSAPtr &ssa) {
        for (const auto &it : users_) {
            auto use = SSACast<UseSSA>(it);
            if (use) {
                // UseSSA pointer
                use->set_ref(ssa);
            }
            else {
                auto phi = SSACast<PhiSSA>(it);
                // Phi node pointer
                if (phi) phi->ReplaceOpr(this, ssa);
            }
        }
    }

    const SSAPtrList &operands() const { return opr_list_; }
    IDType block_id() const { return id_; }

private:
    void ReplaceOpr(PhiSSA *find, SSAPtr &ssa) {
        for (auto &&it : opr_list_) {
            if (it.get() == find) it = ssa;
        }
    }

    SSAPtrList opr_list_;
};

class BlockSSA : public DefSSA {
public:
    BlockSSA(IDType id, SSAPtr body) : DefSSA(id), body_(body) {}

    void AddPred(SSAPtr pred) { preds_.push_back(pred); }
    IDType id() const { return id_; }
    const SSAPtrList &preds() const { return preds_; }

private:
    SSAPtr body_;
    SSAPtrList preds_;
};

class VarDefSSA : public DefSSA {
public:
    VarDefSSA(IDType id) : DefSSA(id) {}
};

class VarUseSSA : public UseSSA {
public:
    VarUseSSA(SSAPtr ref) : UseSSA(ref) {}
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
