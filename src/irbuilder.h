#ifndef SABY_IRBUILDER_H_
#define SABY_IRBUILDER_H_

#include <vector>

#include "ssa.h"

class IRBuilder {
public:
    IRBuilder() : current_block_(0), current_var_(0) {}
    ~IRBuilder() { Release(); }

    std::shared_ptr<BlockSSA> NewBlock();
    std::shared_ptr<VariableSSA> NewVariable(SSAPtr &value);

    void WriteVariable(IDType var_id, IDType block_id, SSAPtr &value);
    SSAPtr ReadVariable(IDType var_id, IDType block_id);
    void SealBlock(SSAPtr &block);

    void Release();

    std::shared_ptr<BlockSSA> GetCurrentBlock() const {
        return blocks_[current_block_];
    }

private:
    SSAPtr ReadVariableRecursive(IDType var_id, IDType block_id);
    SSAPtr AddPhiOperands(IDType var_id, SSAPtr &phi);
    SSAPtr TryRemoveTrivialPhi(const SSAPtr &phi);

    IDType current_block_, current_var_;
    std::vector<SSAPtrList> current_def_, incomplete_phis_;
    std::vector<std::shared_ptr<BlockSSA>> blocks_;
    std::vector<IDType> sealed_blocks_;
};

#endif // SABY_IRBUIDER_H_
