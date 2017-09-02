#include "irbuilder.h"

#include <memory>

SSAPtr IRBuilder::NewBlock(SSAPtrList body) {
    auto block = std::make_unique<BlockSSA>(std::move(body));
    block_var_id_.insert({reinterpret_cast<size_t>(block.get()), 0});
    return std::move(block);
}

unsigned int IRBuilder::GetVariableID(SSAPtr &block, const std::string &var_name) {
    auto block_id = reinterpret_cast<size_t>(block.get());
    unsigned int id = 0;

    // get next available id in this block
    auto var_id = block_var_id_.find(block_id);
    if (var_id == block_var_id_.end()) {
        block_var_id_.insert({block_id, 0});
    }
    else {
        id = var_id->second++;   // increase available id
    }

    auto it = block_var_def_.find(block_id);
    if (it == block_var_def_.end()) {
        block_var_def_.insert({block_id, {{var_name, id}}});
    }
    else {
        auto &var_def = it->second;
        auto vd_it = var_def.find(var_name);
        if (vd_it == var_def.end()) {
            var_def.insert({var_name, id});
        }
        else {
            // variable definition has already existed in current block
            id = vd_it->second;
            --var_id->second;
        }
    }

    return id;
}
