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
protected:
    std::shared_ptr<BaseSSA> next;
};

using SSARef = BaseSSA *;
using SSAPtr = std::shared_ptr<BaseSSA>;
using SSAPtrList = std::vector<SSAPtr>;

class UseSSA : public BaseSSA {
protected:
    SSAPtr reference_;
};

class DefSSA : public BaseSSA {
protected:
    IDType id_;
    SSAPtrList users_;
};

class PhiSSA : public BaseSSA {
public:
    //
private:
    //
};

#endif // SABY_SSA_H_
