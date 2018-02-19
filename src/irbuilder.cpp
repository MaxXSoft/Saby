// reference: Simple and Efficient Construction of Static Single Assignment Form

#include "irbuilder.h"

#include <memory>
#include <utility>
#include <algorithm>

std::shared_ptr<BlockSSA> IRBuilder::NewBlock() {
    current_block_ = block_id_gen_++;
    auto new_block = std::make_shared<BlockSSA>(current_block_);
    blocks_.push_back(new_block);
    current_def_.push_back({});
    incomplete_phis_.push_back({});
    return new_block;
}

std::shared_ptr<VariableSSA> IRBuilder::NewVariable(const IDType &id, SSAPtr value) {
    auto var_ssa = std::make_shared<VariableSSA>(id, value);
    WriteVariable(id, current_block_, var_ssa);
    return var_ssa;
}

void IRBuilder::WriteVariable(const IDType &var_id, BlockIDType block_id, SSAPtr value) {
    assert(block_id <= current_def_.size());
    if (block_id == current_def_.size()) current_def_.push_back({});
    auto &current_var_list = current_def_[block_id];
    current_var_list[var_id] = value;
}

SSAPtr IRBuilder::ReadVariable(const IDType &var_id, BlockIDType block_id) {
    assert(block_id <= current_def_.size());
    const auto &current_var_list = current_def_[block_id];
    auto it = current_var_list.find(var_id);
    if (it != current_var_list.end()) {
        // local value numbering
        return it->second;
    }
    // global value numbering
    return ReadVariableRecursive(var_id, block_id);
}

SSAPtr IRBuilder::ReadVariableRecursive(const IDType &var_id, BlockIDType block_id) {
    SSAPtr value;
    auto it = std::find(sealed_blocks_.begin(), sealed_blocks_.end(), block_id);
    if (it == sealed_blocks_.end()) {
        // incomplete CFG
        auto &current_phi_list = incomplete_phis_[block_id];
        auto phi_it = current_phi_list.find(var_id);
        if (phi_it != current_phi_list.end()) {
            value = phi_it->second;
        }
        else {
            auto phi = std::make_shared<PhiSSA>(block_id);
            value = phi;
            phi->set_ref(phi);
            current_phi_list.insert({var_id, value});
        }
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

SSAPtr IRBuilder::AddPhiOperands(const IDType &var_id, SSAPtr &phi) {
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
    std::vector<User *> users;
    for (const auto &it : phi->uses()) {
        auto user = it->user();
        // remember all users except the phi itself
        if (user != phi_ptr) users.push_back(user);
    }
    // reroute all uses of phi to same
    auto uses = phi->uses();   // copy an use list from phi
    for (const auto &use : uses) {
        use->set_value(same);
    }
    // try to recursively remove all phi users, 
    // which might have become trivial
    for (const auto &user : users) {
        if (user->name()[0] == 'p') {   // user is a phi node
            auto phi_user = SSACast<PhiSSA>(user);
            TryRemoveTrivialPhi(phi_user->ref());
        }
    }
    return same;
}

void IRBuilder::SealBlock(SSAPtr block) {
    auto block_id = SSACast<BlockSSA>(block)->id();
    auto it = std::find(sealed_blocks_.begin(), sealed_blocks_.end(), block_id);
    if (it == sealed_blocks_.end()) {
        auto &phi_list = incomplete_phis_[block_id];
        for (auto &&it : phi_list) {
            AddPhiOperands(it.first, it.second);
        }
        sealed_blocks_.push_back(block_id);
    }
}

void IRBuilder::Release() {
    auto ResetList = [](auto &list) {
        for (auto &&it : list) it.reset();
        list.clear();
    };
    auto ResetMap = [](auto &map) {
        for (auto &&it : map) it.second.reset();
        map.clear();
    };
    auto Reset2DList = [&ResetMap](auto &list) {
        for (auto &&i : list) {
            ResetMap(i);
        }
        list.clear();
    };
    Reset2DList(current_def_);
    Reset2DList(incomplete_phis_);
    ResetList(blocks_);
    pred_value_.reset();
    current_block_ = block_id_gen_ = 0;
}
