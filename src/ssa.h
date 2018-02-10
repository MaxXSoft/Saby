#ifndef SABY_SSA_H_
#define SABY_SSA_H_

#include <memory>
#include <utility>
#include <list>
#include <string>
#include <cstddef>

#if NDEBUG
#define SABY_INLINE inline
#else   // SSACast assertion on debug mode
#define SABY_INLINE
#include <type_traits>
#include <cassert>
#endif

#include "type.h"
#include "def_use.h"

class ValueSSA : public Value {
public:
    ValueSSA(long long value)
            : Value("#num"), num_val_(value), type_(ValueType::Number) {}
    ValueSSA(double value)
            : Value("#dec"), dec_val_(value), type_(ValueType::Decimal) {}
    ValueSSA(const std::string &value)
            : Value("#str"), str_val_(value), type_(ValueType::String) {}

    void Print() override;

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
class ArgGetterSSA : public Value {
public:
    ArgGetterSSA(IDType arg_id) : Value("#arg"), arg_id_(arg_id) {}

    void Print() override;

private:
    IDType arg_id_;
};

// used to represent definition of external function
class ExternFuncSSA : public Value {
public:
    ExternFuncSSA(const std::string &func_name)
            : Value("#ext"), func_name_(func_name) {}

    void Print() override;

private:
    std::string func_name_;
};

// store inline assembly block
// NOTICE: nothing can use it although it's a 'Value'
class AsmSSA : public Value {
public:
    AsmSSA(const std::string &text) : Value("asm:"), text_(text) {}

    void Print() override;

private:
    std::string text_;
};

class UndefSSA : public Value {
public:
    UndefSSA() : Value("#und") {}

    void Print() override;
};

class PhiSSA : public User {
public:
    PhiSSA(IDType block_id) : User("phi"), block_id_(block_id) {}

    void AddOperand(SSAPtr opr) { push_back(Use(opr, this)); }
    void ReplaceBy(SSAPtr &ssa) {
        auto value = static_cast<Value *>(this);
        for (auto &&use : *value) {
            use->set_value(ssa);   // TODO: test
        }
    }

    void Print() override;

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

    void AddPred(SSAPtr pred) { push_back(Use(pred, this)); }
    void AddValue(SSAPtr value) { insts_.push_back(value); }

    void Print() override;

    void set_is_func(bool is_func) { is_func_ = is_func; }

    IDType id() const { return id_; }
    bool is_func() const { return is_func_; }
    const std::list<SSAPtr> &insts() const { return insts_; }

private:
    IDType id_;
    bool is_func_;
    std::list<SSAPtr> insts_;
};

class JumpSSA : public User {
public:
    // NOTICE: 'block' must be a 'BlockSSA'
    JumpSSA(SSAPtr block, SSAPtr cond) : User("jump->") {
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

    void Print() override;
};

class ArgSetterSSA : public User {
public:
    ArgSetterSSA(int arg_pos, SSAPtr value)
            : User("$arg"), arg_pos_(arg_pos) {
        reserve(1);
        push_back(Use(value, this));
    }

    void Print() override;

private:
    int arg_pos_;
};

class CallSSA : public User {
public:
    CallSSA(SSAPtr callee) : User("call->") {
        reserve(kFuncMaxArgNum + 1);
        push_back(Use(callee, this));
    }

    void Print() override;

    void AddArg(SSAPtr arg_setter) { push_back(Use(arg_setter, this)); }
};

class RtnGetterSSA : public User {
public:
    RtnGetterSSA(SSAPtr call) : User("rtn-of") {
        reserve(1);
        push_back(Use(call, this));
    }

    void Print() override;
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

    void Print() override;

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

    void Print() override;

    IDType id() const { return id_; }

private:
    IDType id_;
};

template <typename T>
SABY_INLINE T *SSACast(Value *ptr) {
#if !defined(NDEBUG)   // check if it's a valid cast on debug mode
    const auto &name = ptr->name();
    switch (name[0]) {
        case '#': {
            switch (name[1]) {
                case 'n': case 'd':
                case 's': assert((std::is_same<T, ValueSSA>::value)); break;
                case 'a': assert((std::is_same<T, ArgGetterSSA>::value)); break;
                case 'e': assert((std::is_same<T, ExternFuncSSA>::value)); break;
                case 'u': assert((std::is_same<T, UndefSSA>::value)); break;
            }
            break;
        }
        case '$': {
            switch (name[1]) {
                case 'a': assert((std::is_same<T, ArgSetterSSA>::value)); break;
                case 'v': assert((std::is_same<T, VariableSSA>::value)); break;
            }
            break;
        }
        case 'a': assert((std::is_same<T, AsmSSA>::value)); break;
        case 'p': assert((std::is_same<T, PhiSSA>::value)); break;
        case 'b': assert((std::is_same<T, BlockSSA>::value)); break;
        case 'j': assert((std::is_same<T, JumpSSA>::value)); break;
        case 'c': assert((std::is_same<T, CallSSA>::value)); break;
        case 'r': assert((std::is_same<T, RtnGetterSSA>::value)); break;
        case 'i': assert((std::is_same<T, QuadSSA>::value)); break;
    }
#endif
    return static_cast<T *>(ptr);
}

template <typename T>
inline T *SSACast(const SSAPtr &ptr) {
    return SSACast<T>(ptr.get());
}

#endif // SABY_SSA_H_
