#ifndef SABY_IRBUILDER_H_
#define SABY_IRBUILDER_H_

#include <vector>

#include "ssa.h"

class IRBuilder {
public:
    IRBuilder() : current_block_(0), var_id_({0}) {}
    ~IRBuilder() {}

    SSAPtr NewBlock(SSAPtrList body);
    SSAPtr NewVariable();

    void WriteVariable(unsigned int var_id, unsigned int block_id, SSARef value);
    SSARef ReadVariable();

private:
    using SSARef = BaseSSA *;
    std::vector<unsigned int> var_id_;
    unsigned int current_block_;
};

#endif // SABY_IRBUIDER_H_
