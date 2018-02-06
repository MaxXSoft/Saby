#ifndef SABY_IRBUILDER_H_
#define SABY_IRBUILDER_H_

#include <vector>
#include <utility>
#include <stack>
#include <cassert>

#include "ssa.h"

// break & continue information
using BreakContPair = std::pair<SSAPtr, SSAPtr>;

class IRBuilder {
public:
    IRBuilder() : current_block_(0), block_id_gen_(0), current_var_(0) {}
    ~IRBuilder() { Release(); }

    std::shared_ptr<BlockSSA> NewBlock();
    std::shared_ptr<VariableSSA> NewVariable(SSAPtr value);

    void WriteVariable(IDType var_id, IDType block_id, SSAPtr value);
    SSAPtr ReadVariable(IDType var_id, IDType block_id);
    void SealBlock(SSAPtr block);

    void Release();

    std::shared_ptr<BlockSSA> GetCurrentBlock() const {
        return blocks_[current_block_];
    }

    IDType SwitchCurrentBlock(IDType new_block_id) {
        assert(new_block_id <= block_id_gen_);
        auto cur_id = current_block_;
        current_block_ = new_block_id;
        return cur_id;
    }

    void set_pred_value(SSAPtr pred_value) { pred_value_ = pred_value; }
    const SSAPtr &pred_value() const { return pred_value_; }
    std::stack<BreakContPair> &break_cont_stack() { return break_cont_stack_; }

private:
    SSAPtr ReadVariableRecursive(IDType var_id, IDType block_id);
    SSAPtr AddPhiOperands(IDType var_id, SSAPtr &phi);
    SSAPtr TryRemoveTrivialPhi(const SSAPtr &phi);

    // current_block/var_: store the current block/var id
    // block_id_gen_: generate next block id
    IDType current_block_, block_id_gen_, current_var_;
    SSAPtr pred_value_;
    // used in 'while' generating
    std::stack<BreakContPair> break_cont_stack_;
    std::vector<SSAPtrList> current_def_, incomplete_phis_;
    std::vector<std::shared_ptr<BlockSSA>> blocks_;
    std::vector<IDType> sealed_blocks_;
};

#endif // SABY_IRBUIDER_H_
