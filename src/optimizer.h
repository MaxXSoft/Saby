#ifndef SABY_OPTIMIZER_H_
#define SABY_OPTIMIZER_H_

/*
    TODO: optimizer can do:
      √ constant folding
      √ algebraic simplification
      √ strength reduction
      √ copy propagation & constant propagation
        common subexpression elimination
        remove redundant jump/block
        short-circuit evaluation (need to rewrite genir.cpp)
        dead code elimination (using use-def or not)
        *tail recursive optimization
        *function inlining (optional)
*/

#include "irbuilder.h"

class Optimizer {
public:
    using Operator = QuadSSA::Operator;

    Optimizer(IRBuilder &irb) : irb_(irb), enabled_(true) {}
    ~Optimizer() {}

    // NOTE: method may modify 'lhs' or 'rhs' (auto copy propagation)
    SSAPtr OptimizeBinExpr(Operator op, SSAPtr &lhs, SSAPtr &rhs, int type);
    SSAPtr OptimizeUnaExpr(Operator op, SSAPtr &operand);
    bool OptimizeAssign(SSAPtr &rhs);

    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool enabled() const { return enabled_; }

private:
    template <typename T>
    T CalcValue(Operator op, const T &lhs, const T &rhs);

    SSAPtr ConstFold(Operator op, const SSAPtr &lhs, const SSAPtr &rhs);
    SSAPtr ConstFoldUna(Operator op, const SSAPtr &operand);
    SSAPtr AlgebraSimplify(Operator op, const SSAPtr &lhs, const SSAPtr &rhs, int type);
    SSAPtr StrengthReduct(Operator op, const SSAPtr &lhs, const SSAPtr &rhs, int type);
    SSAPtr CopyProp(const SSAPtr &rhs);
    SSAPtr CSE();
    SSAPtr RemoveRedundantJump();
    SSAPtr DeadCodeElim();

    IRBuilder &irb_;
    bool enabled_;
};

#endif // SABY_OPTIMIZER_H_
