#ifndef SABY_DEF_USE_H_
#define SABY_DEF_USE_H_

// reference: LLVM version 1.3

#include <memory>
#include <utility>
#include <forward_list>
#include <vector>
#include <string>
#include <cstddef>
#include <cassert>

class Value;
class User;
class Use;

using SSAPtr = std::shared_ptr<Value>;
using SSAPtrList = std::vector<SSAPtr>;

class Value {
public:
    // in order to print or debug the use-def chain
    Value(const std::string &name) : name_(name) {}
    virtual ~Value() = default;

    void AddUse(Use *use) { uses_.push_front(use); }
    void RemoveUse(Use *use) { uses_.remove(use); }

    virtual void Print() = 0;

    const std::forward_list<Use *> &uses() const { return uses_; }
    const std::string &name() const { return name_; }

private:
    std::forward_list<Use *> uses_;   // singly-linked list for Use
    std::string name_;
};

class Use {
public:
    explicit Use(SSAPtr value, User *user)
            : copied_(false), value_(value), user_(user) {}
    // copy constructor
    Use(const Use &use)
            : copied_(true), value_(use.value_), user_(use.user_) {
        if (value_) value_->AddUse(this);
    }
    ~Use() { if (copied_ && value_) value_->RemoveUse(this); }

    void set_value(const SSAPtr &value) {
        assert(copied_);
        if (value_) value_->RemoveUse(this);
        value_ = value;
        if (value_) value_->AddUse(this);
    }

    SSAPtr value() const { return value_; }
    User *user() const { return user_; }

private:
    bool copied_;
    SSAPtr value_;   // User --Use-> Value
    User *user_;
};

class User : public Value {
public:
    using OpIter = std::vector<Use>::iterator;

    explicit User(const std::string &name) : Value(name) {}

    void reserve(std::size_t size) { operands_.reserve(size); }
    void push_back(SSAPtr value) { operands_.push_back(Use(value, this)); }

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
