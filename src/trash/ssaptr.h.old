#ifndef SABY_SSAPTR_H_
#define SABY_SSAPTR_H_

#include <map>
#include <utility>
#include <cassert>

class Value;   // def_use.h -> Value

class ObjectWrapper {
public:
    ObjectWrapper(Value *value) : value_(value), counter_(1) {}
    ~ObjectWrapper() { Release(); }

    void IncreaseCounter() { ++counter_; }

    // return true if value_ is not released
    bool DecreaseCounter() {
        if (counter_ && --counter_ == 0) Release();
        return counter_;
    }

    void Release() {
        if (value_) {
            delete value_;
            value_ = nullptr;
        }
    }

    bool Unique() const { return value_ && (counter_ == 1); }
    Value *value() const { return value_; }

private:
    Value *value_;
    long counter_;
};

class PtrManager {
public:
    PtrManager() {}
    ~PtrManager() {}

    void AddPtr(Value *value, ObjectWrapper *wrapper) {
        if (value && wrapper) ptrs_[value] = wrapper;
    }

    // suppose that can always find 'value' in 'ptrs_'
    ObjectWrapper *GetPtr(Value *value) { return ptrs_[value]; }

    void DelPtr(Value *value) {
        auto it = ptrs_.find(value);
        if (it != ptrs_.end()) ptrs_.erase(it);
    }

private:
    std::map<Value *, ObjectWrapper *> ptrs_;
};

class SSAPtr {
public:
    explicit SSAPtr(PtrManager *pm, Value *value)
            : pm_(pm), wrapper_(new ObjectWrapper(value)) {
        if (pm_) pm_->AddPtr(value, wrapper_);
    }

    SSAPtr() : pm_(nullptr), wrapper_(nullptr) {}
    SSAPtr(std::nullptr_t) : pm_(nullptr), wrapper_(nullptr) {}

    SSAPtr(const SSAPtr &ssa) : pm_(ssa.pm_), wrapper_(ssa.wrapper_) {
        if (wrapper_) wrapper_->IncreaseCounter();
    }

    SSAPtr(SSAPtr &&ssa) noexcept
            : pm_(ssa.pm_), wrapper_(ssa.wrapper_) {
        ssa.pm_ = nullptr;
        ssa.wrapper_ = nullptr;
    }

    ~SSAPtr() { DecAndRelease(); }

    SSAPtr &operator=(const SSAPtr &ssa) {
        if (this != &ssa) {
            if (ssa.wrapper_) ssa.wrapper_->IncreaseCounter();
            DecAndRelease();
            pm_ = ssa.pm_;
            wrapper_ = ssa.wrapper_;
        }
        return *this;
    }

    SSAPtr &operator=(Value *value) {
        assert(pm_);   // suppose that this pointer has a PtrManager
        if (value != wrapper_->value()) {
            // decrease the ref-count of this pointer
            DecAndRelease();
            // find ObjectWrapper of 'value' and replace
            wrapper_ = pm_->GetPtr(value);
            // increase the ref-count of new pointer
            wrapper_->IncreaseCounter();
        }
        return *this;
    }

    SSAPtr &operator=(SSAPtr &&ssa) noexcept {
        if (this != &ssa) {
            pm_ = ssa.pm_;
            wrapper_ = ssa.wrapper_;
            ssa.pm_ = nullptr;
            ssa.wrapper_ = nullptr;
        }
        return *this;
    }

    Value &operator*() const noexcept { return *wrapper_->value(); }
    Value *operator->() const noexcept { return wrapper_->value(); }

    explicit operator bool() const noexcept { return wrapper_ && wrapper_->value(); }
    friend bool operator==(const SSAPtr &lhs, const SSAPtr &rhs) { return lhs.wrapper_ == rhs.wrapper_; }
    friend bool operator!=(const SSAPtr &lhs, const SSAPtr &rhs) { return lhs.wrapper_ != rhs.wrapper_; }
    friend bool operator==(const SSAPtr &lhs, std::nullptr_t rhs) { return lhs.wrapper_ == nullptr; }
    friend bool operator!=(const SSAPtr &lhs, std::nullptr_t rhs) { return lhs.wrapper_ != nullptr; }
    friend bool operator==(std::nullptr_t lhs, const SSAPtr &rhs) { return rhs.wrapper_ == nullptr; }
    friend bool operator!=(std::nullptr_t lhs, const SSAPtr &rhs) { return rhs.wrapper_ != nullptr; }

    void set_pm(PtrManager *pm) { if (!wrapper_) pm_ = pm; }
    void reset() {
        DecAndRelease();
        wrapper_ = nullptr;
    }

    Value *get() const { return wrapper_ ? wrapper_->value() : nullptr; }
    bool unique() const { return wrapper_ ? wrapper_->Unique() : false; }

    template <typename T>
    T *Cast() const { return dynamic_cast<T *>(get()); }

private:
    void DecAndRelease() {
        auto value = get();
        if (wrapper_ && !wrapper_->DecreaseCounter()) {
            if (pm_) pm_->DelPtr(value);
            delete wrapper_;
            wrapper_ = nullptr;
        }
    }

    PtrManager *pm_;
    ObjectWrapper *wrapper_;
};

template <typename T, typename... Args>
inline SSAPtr MakeSSA(PtrManager *pm, Args&&... args) {
    return SSAPtr(pm, new T(std::forward<Args>(args)...));
}

#endif // SABY_SSAPTR_H_
