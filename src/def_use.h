#ifndef SABY_DEF_USE_H_
#define SABY_DEF_USE_H_

// reference: LLVM version 1.3

#include <memory>
#include <list>
#include <vector>
#include <string>
#include <cstddef>

class Value;
class User;

using SSAPtr = std::shared_ptr<Value>;
using SSAPtrList = std::vector<SSAPtr>;

template <typename T>
inline T *SSACast(const SSAPtr &ptr) {
    return dynamic_cast<T *>(ptr.get());
}

template <typename T>
inline T *SSACast(Value *ptr) {
    return dynamic_cast<T *>(ptr);
}

class Use {
public:
    Use(SSAPtr value, User *user) : value_(value), user_(user) {
        if (value_) value_->AddUse(this);
    }
    ~Use() { if (value_) value_->RemoveUse(this); }

    void set_value(SSAPtr &value) {
        if (value_) value_->RemoveUse(this);
        value_ = value;
        if (value_) value_->AddUse(this);
    }

    SSAPtr value() const { return value_; }
    User *user() const { return user_; }

private:
    SSAPtr value_;   // User --Use-> Value
    User *user_;
};

class Value {
public:
    using ListIter = std::list<Use *>::iterator;

    // in order to print or debug the use-def chain
    Value(const std::string &name) : name_(name) {}
    virtual ~Value() = default;

    // TODO: whether to allow duplicate elements?
    // Thinking: allow, consider the following situation
    //     var a = 1; var b = a + a
    void AddUse(Use *use) { uses_.push_back(use); }
    // TODO: consider moving the impl to a separate file
    // TODO: consider if can cause memory error
    void RemoveUse(Use *use) {
        for (auto it = uses_.begin(); it != uses_.end(); ) {
            if (*it == use) {
                it = uses_.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // virtual void Generate() = 0;

    ListIter begin() { return uses_.begin(); }
    ListIter end() { return uses_.end(); }
    ListIter erase(ListIter it) { return uses_.erase(it); }

    std::size_t size() const { return uses_.size(); }
    bool empty() const { return uses_.empty(); }
    const std::string &name() const { return name_; }

private:
    std::list<Use *> uses_;   // doubly-linked list for Use
    std::string name_;
};

class User : public Value {
public:
    using OpIter = std::vector<Use>::iterator;

    User(const std::string &name) : Value(name) {}

    void reserve(std::size_t size) { operands_.reserve(size); }
    void push_back(Use &&use) { operands_.push_back(use); }

    Use &operator[](std::size_t pos) { return operands_[pos]; }
    const Use &operator[](std::size_t pos) const { return operands_[pos]; }

    OpIter begin() { return operands_.begin(); }
    OpIter end() { return operands_.end(); }
    std::size_t size() const { return operands_.size(); }
    bool empty() const { return operands_.empty(); }

private:
    std::vector<Use> operands_;
};

#endif // SABY_DEF_USE_H_
