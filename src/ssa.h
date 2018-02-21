#ifndef SABY_SSA_H_
#define SABY_SSA_H_

#include <memory>
#include <utility>
#include <list>
#include <string>
#include <cstddef>
#include <cassert>

#if NDEBUG
#define SABY_INLINE inline
#else   // SSACast assertion on debug mode
#define SABY_INLINE
#include <type_traits>
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
    ArgGetterSSA(int arg_id) : Value("#arg"), arg_id_(arg_id) {}

    void Print() override;

private:
    int arg_id_;
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
    PhiSSA(BlockIDType block_id) : User("phi"), block_id_(block_id) {}

    void AddOperand(SSAPtr opr) {
        if (opr.get() != this) {
            if (opr->name()[0] == 'p') {   // if operand is a 'PhiSSA'
                auto phi_ptr = static_cast<PhiSSA *>(opr.get());
                for (const auto &use : *phi_ptr) {
                    push_back(use.value());
                }
            }
            else {
                push_back(opr);
            }
        }
    }

    void Print() override;

    void set_ref(const std::shared_ptr<PhiSSA> &phi) { ref_ = phi; }
    SSAPtr ref() { return ref_.lock(); }

    BlockIDType block_id() const { return block_id_; }

private:
    BlockIDType block_id_;
    std::weak_ptr<PhiSSA> ref_;
};

class BlockSSA : public User {
public:
    BlockSSA(BlockIDType id)
            : User("block:"), id_(id), is_func_(false) {}

    void AddPred(SSAPtr pred) { push_back(pred); }
    void AddValue(SSAPtr value) { insts_.push_back(value); }

    void Print() override;

    void set_is_func(bool is_func) { is_func_ = is_func; }

    BlockIDType id() const { return id_; }
    bool is_func() const { return is_func_; }
    const std::list<SSAPtr> &insts() const { return insts_; }

private:
    BlockIDType id_;
    bool is_func_;
    std::list<SSAPtr> insts_;
};

class JumpSSA : public User {
public:
    // NOTICE: 'block' must be a 'BlockSSA'
    JumpSSA(SSAPtr block, SSAPtr cond) : User("jump->") {
        if (cond) {
            reserve(2);
            push_back(block);
            push_back(cond);
        }
        else {
            reserve(1);
            push_back(block);
        }
    }

    void Print() override;
};

class ArgSetterSSA : public User {
public:
    ArgSetterSSA(int arg_pos, SSAPtr value)
            : User("$arg"), arg_pos_(arg_pos) {
        reserve(1);
        push_back(value);
    }

    void Print() override;

private:
    int arg_pos_;
};

class CallSSA : public User {
public:
    CallSSA(SSAPtr callee) : User("call->") {
        reserve(kFuncMaxArgNum + 1);
        push_back(callee);
    }

    void Print() override;

    void AddArg(SSAPtr arg_setter) { push_back(arg_setter); }
};

class RtnGetterSSA : public User {
public:
    RtnGetterSSA(SSAPtr call) : User("rtn-of") {
        reserve(1);
        push_back(call);
    }

    void Print() override;
};

class ReturnSSA : public User {
public:
    ReturnSSA(SSAPtr value) : User("ret") {
        if (value) {
            reserve(1);
            push_back(value);
        }
        else {
            reserve(0);
        }
    }

    void Print() override;
};

class QuadSSA : public User {
public:
    enum class Operator : char {
        ConvNum, ConvDec, ConvStr,
        And, Xor, Or, Not, Shl, Shr,
        Add, Sub, Mul, Div, Mod, Pow,
        Less, LessEqual, Greater, GreaterEqual, Euqal, NotEqual
    };

    QuadSSA(Operator op, SSAPtr opr1, SSAPtr opr2) : User("inst"), op_(op) {
        if (opr2) {
            reserve(2);
            push_back(opr1);
            push_back(opr2);
        }
        else {
            reserve(1);
            push_back(opr1);
        }
    }

    void Print() override;

private:
    Operator op_;
};

class VariableSSA : public User {
public:
    VariableSSA(const IDType &id, SSAPtr value) : User("$var"), id_(id) {
        assert(value);
        reserve(1);
        push_back(value);
    }

    void Print() override;

    const IDType &id() const { return id_; }

private:
    IDType id_;
};

// SFINAE definitions for function template 'IsSSAType'
template <typename T> inline bool IsSSAType(Value *ptr) { return false; }
template <typename T> inline bool IsSSAType(const SSAPtr &ptr) { return IsSSAType<T>(ptr.get()); }
template <> inline bool IsSSAType<ValueSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == '#' && (name[1] == 'n' || name[1] == 'd' || name[1] == 's');
}
template <> inline bool IsSSAType<ArgGetterSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == '#' && name[1] == 'a';
}
template <> inline bool IsSSAType<ExternFuncSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == '#' && name[1] == 'e';
}
template <> inline bool IsSSAType<UndefSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == '#' && name[1] == 'u';
}
template <> inline bool IsSSAType<ArgSetterSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == '$' && name[1] == 'a';
}
template <> inline bool IsSSAType<VariableSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == '$' && name[1] == 'v';
}
template <> inline bool IsSSAType<RtnGetterSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'r' && name[1] == 't';
}
template <> inline bool IsSSAType<ReturnSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'r' && name[1] == 'e';
}
template <> inline bool IsSSAType<AsmSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'a';
}
template <> inline bool IsSSAType<PhiSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'p';
}
template <> inline bool IsSSAType<BlockSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'b';
}
template <> inline bool IsSSAType<JumpSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'j';
}
template <> inline bool IsSSAType<CallSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'c';
}
template <> inline bool IsSSAType<QuadSSA>(Value *ptr) {
    const auto &name = ptr->name();
    return name[0] == 'i';
}
// end SFINAE definitions

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
        case 'r': {
            switch (name[1]) {
                case 't': assert((std::is_same<T, RtnGetterSSA>::value)); break;
                case 'e': assert((std::is_same<T, ReturnSSA>::value)); break;
            }
            break;
        }
        case 'a': assert((std::is_same<T, AsmSSA>::value)); break;
        case 'p': assert((std::is_same<T, PhiSSA>::value)); break;
        case 'b': assert((std::is_same<T, BlockSSA>::value)); break;
        case 'j': assert((std::is_same<T, JumpSSA>::value)); break;
        case 'c': assert((std::is_same<T, CallSSA>::value)); break;
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
