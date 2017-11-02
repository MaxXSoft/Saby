#ifndef SABY_IRBUILDER_H_
#define SABY_IRBUILDER_H_

#include <vector>

#include "ssa.h"

class IRBuilder {
public:
    IRBuilder() : current_block_(0) /*, current_def_({{nullptr}}) */ {}
    ~IRBuilder() {}

    SSAPtr NewBlock(SSAPtrList body);
    SSAPtr NewVariable();

    void WriteVariable(IDType var_id, IDType block_id, SSAPtr value);
    SSARef ReadVariable(IDType var_id, IDType block_id);
    SSARef ReadVariableRecursive(IDType var_id, IDType block_id);
    SSARef AddPhiOperands(IDType var_id, SSAPtr &phi);

    void Release();

private:
    IDType current_block_;
    std::vector<SSAPtrList> current_def_, incomplete_phis_;
    std::vector<std::unique_ptr<BlockSSA>> blocks_;
    std::vector<IDType> sealed_blocks_;
};

#endif // SABY_IRBUIDER_H_
