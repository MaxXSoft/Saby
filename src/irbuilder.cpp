// reference: Simple and Efficient Construction of Static Single Assignment Form

#include "irbuilder.h"

#include <memory>
#include <utility>
#include <algorithm>

namespace {

// BlockSSA *BlockSSACast(SSAPtr &ptr) {
//     return dynamic_cast<BlockSSA *>(ptr.get());
// }

}

// SSAPtr IRBuilder::NewBlock(SSAPtrList body) {
//     auto block = std::make_unique<BlockSSA>(current_block_++, std::move(body));
//     var_id_.push_back(0);
//     return std::move(block);
// }

// SSAPtr IRBuilder::NewVariable() {
//     return std::make_unique<VariableSSA>(var_id_[current_block_]++);
// }

void IRBuilder::WriteVariable(IDType var_id, IDType block_id, SSAPtr value) {
    if (block_id < current_def_.size() && var_id < current_def_[block_id].size()) {
        current_def_[block_id][var_id] = std::move(value);
    }
}

SSARef IRBuilder::ReadVariable(IDType var_id, IDType block_id) {
    if (var_id < current_def_[block_id].size()) {
        // local value numbering
        return current_def_[block_id][var_id].get();
    }
    // global value numbering
    return ReadVariableRecursive(var_id, block_id);
}

SSARef IRBuilder::ReadVariableRecursive(IDType var_id, IDType block_id) {
    SSARef value;
    auto it = std::find(sealed_blocks_.begin(), sealed_blocks_.end(), block_id);
    if (it == sealed_blocks_.end()) {
        // incomplete CFG
        auto phi = std::make_unique<PhiSSA>(block_id);
        value = phi.get();
        incomplete_phis_[block_id][var_id] = std::move(phi);
    }
    else if (blocks_[block_id]->preds().size() == 1) {
        // optimize the common case of one predecessor: no phi needed
        auto ptr = blocks_[block_id]->preds()[0].get();
        value = ReadVariable(var_id, dynamic_cast<BlockSSA *>(ptr)->id());
    }
    else {
        // break potential cycles with operandless phi
        auto phi = std::make_unique<PhiSSA>(block_id);
        WriteVariable(var_id, block_id, std::move(phi));
        //value = AddPhiOperands();
    }
}

SSARef AddPhiOperands(IDType var_id, SSAPtr &phi) {
    //
}

