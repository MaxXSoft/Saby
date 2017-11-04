// reference: Simple and Efficient Construction of Static Single Assignment Form

#include "irbuilder.h"

#include <memory>
#include <utility>
#include <algorithm>

namespace {

template <typename T>
inline T *SSACast(const SSAPtr &ptr) {
    return dynamic_cast<T *>(ptr.get());
}

}

// SSAPtr IRBuilder::NewBlock(SSAPtrList body) {
//     auto block = std::make_shared<BlockSSA>(current_block_++, std::move(body));
//     var_id_.push_back(0);
//     return std::move(block);
// }

// SSAPtr IRBuilder::NewVariable() {
//     return std::make_shared<VariableSSA>(var_id_[current_block_]++);
// }

void IRBuilder::WriteVariable(IDType var_id, IDType block_id, SSAPtr &value) {
    if (block_id < current_def_.size() && var_id < current_def_[block_id].size()) {
        current_def_[block_id][var_id] = value;
    }
}

SSAPtr IRBuilder::ReadVariable(IDType var_id, IDType block_id) {
    if (var_id < current_def_[block_id].size()) {
        // local value numbering
        return current_def_[block_id][var_id];
    }
    // global value numbering
    return ReadVariableRecursive(var_id, block_id);
}

SSAPtr IRBuilder::ReadVariableRecursive(IDType var_id, IDType block_id) {
    SSAPtr value;
    auto it = std::find(sealed_blocks_.begin(), sealed_blocks_.end(), block_id);
    if (it == sealed_blocks_.end()) {
        // incomplete CFG
        value = std::make_shared<PhiSSA>(block_id);
        incomplete_phis_[block_id][var_id] = value;
    }
    else if (blocks_[block_id]->preds().size() == 1) {
        // optimize the common case of one predecessor: no phi needed
        auto pred_0 = blocks_[block_id]->preds()[0];
        value = ReadVariable(var_id, SSACast<BlockSSA>(pred_0)->id());
    }
    else {
        // break potential cycles with operandless phi
        value = std::make_shared<PhiSSA>(block_id);
        WriteVariable(var_id, block_id, value);
        value = AddPhiOperands(var_id, value);
    }
    WriteVariable(var_id, block_id, value);
    return value;
}

SSAPtr IRBuilder::AddPhiOperands(IDType var_id, SSAPtr &phi) {
    auto phi_ptr = SSACast<PhiSSA>(phi);
    auto block_id = phi_ptr->block_id();
    auto preds = blocks_[block_id]->preds();
    // determine operands from predecessors
    for (const auto &pred : preds) {
        auto block_ptr = SSACast<BlockSSA>(pred);
        phi_ptr->AddOpr(ReadVariable(var_id, block_ptr->id()));
    }
    return TryRemoveTrivialPhi(phi);
}

SSAPtr IRBuilder::TryRemoveTrivialPhi(const SSAPtr &phi) {
    SSAPtr same = nullptr;
    auto phi_ptr = SSACast<PhiSSA>(phi);
    for (const auto &op : phi_ptr->operands()) {
        // unique value or self−reference
        if (op == same || op == phi) continue;
        // the phi merges at least two values: not trivial
        if (same != nullptr) return phi;
        same = op;
    }
    // the phi is unreachable or in the start block
    if (same == nullptr) same = std::make_shared<UndefSSA>();
    // remember all users except the phi itself
    auto users = phi_ptr->users();
    for (auto &&it : users) {
        if (it == phi) {
            it = users.back();
            users.pop_back();
            break;
        }
    }
    // reroute all uses of phi to same and remove phi
    phi_ptr->ReplaceBy(same);
    // try to recursively remove all phi users, 
    // which might have become trivial
    for (const auto &use : users) {
        if (SSACast<PhiSSA>(use) != nullptr) TryRemoveTrivialPhi(use);
    }
    return same;
}

void IRBuilder::SealBlock(SSAPtr &block) {
    auto block_id = SSACast<BlockSSA>(block)->id();
    for (const auto &phi : incomplete_phis_[block_id]) {
        // TODO
    }
}

void IRBuilder::Release() {
    //
}
