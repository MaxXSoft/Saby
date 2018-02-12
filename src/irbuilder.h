#ifndef SABY_IRBUILDER_H_
#define SABY_IRBUILDER_H_

#include <vector>
#include <map>
#include <stack>
#include <list>
#include <utility>
#include <cassert>

#include "ssa.h"
#include "type.h"

// break & continue information
using BreakContPair = std::pair<SSAPtr, SSAPtr>;

class IRBuilder {
public:
    IRBuilder() : current_block_(0), block_id_gen_(0) {}
    ~IRBuilder() { Release(); }

    std::shared_ptr<BlockSSA> NewBlock();
    std::shared_ptr<VariableSSA> NewVariable(const IDType &id, SSAPtr value);

    void WriteVariable(const IDType &var_id, BlockIDType block_id, SSAPtr value);
    SSAPtr ReadVariable(const IDType &var_id, BlockIDType block_id);
    void SealBlock(SSAPtr block);

    void Release();

    std::shared_ptr<BlockSSA> GetCurrentBlock() const {
        return blocks_[current_block_];
    }

    BlockIDType SwitchCurrentBlock(BlockIDType new_block_id) {
        assert(new_block_id <= block_id_gen_);
        auto cur_id = current_block_;
        current_block_ = new_block_id;
        return cur_id;
    }

    void set_pred_value(SSAPtr pred_value) { pred_value_ = pred_value; }
    void set_exported_funcs(const LibList &exported_funcs) { exported_funcs_ = exported_funcs; }

    const SSAPtr &pred_value() const { return pred_value_; }
    std::stack<BreakContPair> &break_cont_stack() { return break_cont_stack_; }
    const std::vector<std::shared_ptr<BlockSSA>> &blocks() const { return blocks_; }
    LibList &imported_libs() { return imported_libs_; }

private:
    using SSAPtrMap = std::map<IDType, SSAPtr>;

    SSAPtr ReadVariableRecursive(const IDType &var_id, BlockIDType block_id);
    SSAPtr AddPhiOperands(const IDType &var_id, SSAPtr &phi);
    SSAPtr TryRemoveTrivialPhi(const SSAPtr &phi);

    // current_block/var_: store the current block/var id
    // block_id_gen_: generate next block id
    BlockIDType current_block_, block_id_gen_;
    SSAPtr pred_value_;
    // used in 'while' generating
    std::stack<BreakContPair> break_cont_stack_;
    // info of defs & blocks & phis
    // TODO: consider use map<ID, map<ID, Ptr>> to store defs & phis
    std::vector<SSAPtrMap> current_def_, incomplete_phis_;
    std::vector<std::shared_ptr<BlockSSA>> blocks_;
    std::vector<BlockIDType> sealed_blocks_;
    // library info
    LibList imported_libs_, exported_funcs_;
};

#endif // SABY_IRBUIDER_H_
