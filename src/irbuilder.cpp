// reference: Simple and Efficient Construction of Static Single Assignment Form

// TODO: rewrite this file

#include "irbuilder.h"

#include <memory>
#include <utility>
#include <algorithm>

namespace {

//

}

std::shared_ptr<BlockSSA> IRBuilder::NewBlock() {
    return std::make_shared<BlockSSA>(current_block_++);
}

std::shared_ptr<VariableSSA> IRBuilder::NewVariable(SSAPtr value) {
    auto var_id = current_var_++;
    WriteVariable(var_id, current_block_, value);
    return std::make_shared<VariableSSA>(var_id, value);
}

void IRBuilder::WriteVariable(IDType var_id, IDType block_id, SSAPtr value) {
    if (block_id < current_def_.size()) {
        auto &current_var_list = current_def_[block_id];
        if (var_id < current_var_list.size()) {
            current_var_list[var_id] = value;
        }
        else if (var_id == current_var_list.size()) {
            current_var_list.push_back(value);
        }
    }
    else if (block_id == current_def_.size()) {
        current_def_.push_back({});
        WriteVariable(var_id, block_id, value);
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
        auto phi = std::make_shared<PhiSSA>(block_id);
        value = phi;
        phi->set_ref(phi);
        incomplete_phis_[block_id][var_id] = value;
    }
    else if (blocks_[block_id]->size() == 1) {
        // optimize the common case of one predecessor: no phi needed
        auto pred_0 = (*blocks_[block_id])[0].value();
        value = ReadVariable(var_id, SSACast<BlockSSA>(pred_0)->id());
    }
    else {
        // break potential cycles with operandless phi
        auto phi = std::make_shared<PhiSSA>(block_id);
        value = phi;
        phi->set_ref(phi);
        WriteVariable(var_id, block_id, value);
        value = AddPhiOperands(var_id, value);
    }
    WriteVariable(var_id, block_id, value);
    return value;
}

SSAPtr IRBuilder::AddPhiOperands(IDType var_id, SSAPtr &phi) {
    auto phi_ptr = SSACast<PhiSSA>(phi);
    auto block_id = phi_ptr->block_id();
    auto preds = *blocks_[block_id];
    // determine operands from predecessors
    for (const auto &pred : preds) {
        auto block_ptr = SSACast<BlockSSA>(pred.value());
        phi_ptr->AddOperand(ReadVariable(var_id, block_ptr->id()));
    }
    return TryRemoveTrivialPhi(phi);
}

SSAPtr IRBuilder::TryRemoveTrivialPhi(const SSAPtr &phi) {
    SSAPtr same = nullptr;
    auto phi_ptr = SSACast<PhiSSA>(phi);
    for (const auto &op : *phi_ptr) {
        // unique value or self−reference
        auto value = op.value();
        if (value == same || value == phi) continue;
        // the phi merges at least two values: not trivial
        if (same != nullptr) return phi;
        same = value;
    }
    // the phi is unreachable or in the start block
    if (same == nullptr) same = std::make_shared<UndefSSA>();
    // remember all users except the phi itself
    std::vector<User *> users;
    for (const auto &it : *phi) {
        auto user = it->user();
        if (user != phi_ptr) users.push_back(user);
    }
    // reroute all uses of phi to same and remove phi
    phi_ptr->ReplaceBy(same);
    // try to recursively remove all phi users, 
    // which might have become trivial
    for (const auto &user : users) {
        // TODO: test
        auto phi_user = SSACast<PhiSSA>(user);
        if (phi_user) TryRemoveTrivialPhi(phi_user->ref());
    }
    return same;
}

void IRBuilder::SealBlock(SSAPtr block) {
    auto block_id = SSACast<BlockSSA>(block)->id();
    auto it = std::find(sealed_blocks_.begin(), sealed_blocks_.end(), block_id);
    if (it == sealed_blocks_.end()) {
        auto &phi_list = incomplete_phis_[block_id];
        // TODO: test
        for (int id = 0; id < phi_list.size(); ++id) {
            AddPhiOperands(id, phi_list[id]);
        }
        sealed_blocks_.push_back(block_id);
    }
}

void IRBuilder::Release() {
    auto ResetList = [](auto &list) {
        for (auto &&it : list) it.reset();
        list.clear();
    };
    auto Reset2DList = [&ResetList](auto &list) {
        for (auto &&i : list) {
            ResetList(i);
        }
        list.clear();
    };
    Reset2DList(current_def_);
    Reset2DList(incomplete_phis_);
    ResetList(blocks_);
    pred_value_.reset();
    current_block_ = current_var_ = 0;
}
